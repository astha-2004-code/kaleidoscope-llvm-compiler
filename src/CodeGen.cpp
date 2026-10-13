#include "kaleidoscope/CodeGen.hpp"
#include <sstream>
#include <cassert>

#if KALEIDOSCOPE_HAS_LLVM
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_os_ostream.h>
#endif

namespace kaleidoscope {

#if KALEIDOSCOPE_HAS_LLVM

IRGenerator::IRGenerator(const std::string &moduleName, DiagnosticEngine *diag)
    : moduleName_(moduleName),
      diag_(diag),
      builder_(context_) {
    module_ = std::make_unique<llvm::Module>(moduleName_, context_);
}

IRGenerator::~IRGenerator() = default;

void IRGenerator::resetModule(const std::string &moduleName) {
    moduleName_ = moduleName;
    module_ = std::make_unique<llvm::Module>(moduleName_, context_);
    namedValues_.clear();
}

llvm::Function *IRGenerator::getFunction(const std::string &name) {
    if (auto *f = module_->getFunction(name)) {
        return f;
    }

    auto fi = functionProtos_.find(name);
    if (fi != functionProtos_.end()) {
        return codegen(*fi->second);
    }

    return nullptr;
}

llvm::Value *IRGenerator::codegen(ExprAST &expr) {
    SourceLocation loc = expr.getLocation();

    if (auto *num = dynamic_cast<NumberExprAST *>(&expr)) {
        return llvm::ConstantFP::get(context_, llvm::APFloat(num->getValue()));
    }

    if (auto *var = dynamic_cast<VariableExprAST *>(&expr)) {
        llvm::Value *v = namedValues_[var->getName()];
        if (!v && diag_) {
            diag_->error(loc, "Unknown variable name '" + var->getName() + "'");
        }
        return v;
    }

    if (auto *unary = dynamic_cast<UnaryExprAST *>(&expr)) {
        llvm::Value *operandV = codegen(*const_cast<ExprAST *>(unary->getOperand()));
        if (!operandV) return nullptr;

        if (unary->getOpcode() == '-') {
            return builder_.CreateFNeg(operandV, "negtmp");
        }
        return operandV; // unary '+' is no-op
    }

    if (auto *bin = dynamic_cast<BinaryExprAST *>(&expr)) {
        llvm::Value *lhsV = codegen(*const_cast<ExprAST *>(bin->getLHS()));
        llvm::Value *rhsV = codegen(*const_cast<ExprAST *>(bin->getRHS()));
        if (!lhsV || !rhsV) return nullptr;

        switch (bin->getOp()) {
            case '+': return builder_.CreateFAdd(lhsV, rhsV, "addtmp");
            case '-': return builder_.CreateFSub(lhsV, rhsV, "subtmp");
            case '*': return builder_.CreateFMul(lhsV, rhsV, "multmp");
            case '/': return builder_.CreateFDiv(lhsV, rhsV, "divtmp");
            case '<': {
                llvm::Value *cmp = builder_.CreateFCmpULT(lhsV, rhsV, "cmptmp");
                return builder_.CreateUIToFP(cmp, llvm::Type::getDoubleTy(context_), "booltmp");
            }
            case '>': {
                llvm::Value *cmp = builder_.CreateFCmpUGT(lhsV, rhsV, "cmptmp");
                return builder_.CreateUIToFP(cmp, llvm::Type::getDoubleTy(context_), "booltmp");
            }
            default:
                if (diag_) {
                    diag_->error(loc, std::string("Invalid binary operator '") + bin->getOp() + "'");
                }
                return nullptr;
        }
    }

    if (auto *call = dynamic_cast<CallExprAST *>(&expr)) {
        llvm::Function *calleeF = getFunction(call->getCallee());
        if (!calleeF) {
            if (diag_) diag_->error(loc, "Unknown function referenced: '" + call->getCallee() + "'");
            return nullptr;
        }

        if (calleeF->arg_size() != call->getArgs().size()) {
            if (diag_) {
                diag_->error(loc, "Incorrect number of arguments passed to '" + call->getCallee() + "'");
            }
            return nullptr;
        }

        std::vector<llvm::Value *> argsV;
        for (auto &arg : const_cast<std::vector<std::unique_ptr<ExprAST>> &>(call->getArgs())) {
            argsV.push_back(codegen(*arg));
            if (!argsV.back()) return nullptr;
        }

        return builder_.CreateCall(calleeF, argsV, "calltmp");
    }

    if (auto *ifExpr = dynamic_cast<IfExprAST *>(&expr)) {
        llvm::Value *condV = codegen(*const_cast<ExprAST *>(ifExpr->getCond()));
        if (!condV) return nullptr;

        condV = builder_.CreateFCmpONE(condV, llvm::ConstantFP::get(context_, llvm::APFloat(0.0)), "ifcond");

        llvm::Function *theFunction = builder_.GetInsertBlock()->getParent();

        llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(context_, "then", theFunction);
        llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(context_, "else");
        llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(context_, "ifcont");

        builder_.CreateCondBr(condV, thenBB, elseBB);

        // Emit then block
        builder_.SetInsertPoint(thenBB);
        llvm::Value *thenV = codegen(*const_cast<ExprAST *>(ifExpr->getThen()));
        if (!thenV) return nullptr;
        builder_.CreateBr(mergeBB);
        thenBB = builder_.GetInsertBlock();

        // Emit else block
        theFunction->getBasicBlockList().push_back(elseBB);
        builder_.SetInsertPoint(elseBB);
        llvm::Value *elseV = codegen(*const_cast<ExprAST *>(ifExpr->getElse()));
        if (!elseV) return nullptr;
        builder_.CreateBr(mergeBB);
        elseBB = builder_.GetInsertBlock();

        // Emit merge block with PHI node
        theFunction->getBasicBlockList().push_back(mergeBB);
        builder_.SetInsertPoint(mergeBB);
        llvm::PHINode *pn = builder_.CreatePHI(llvm::Type::getDoubleTy(context_), 2, "iftmp");
        pn->addIncoming(thenV, thenBB);
        pn->addIncoming(elseV, elseBB);
        return pn;
    }

    if (auto *forExpr = dynamic_cast<ForExprAST *>(&expr)) {
        llvm::Value *startV = codegen(*const_cast<ExprAST *>(forExpr->getStart()));
        if (!startV) return nullptr;

        llvm::Function *theFunction = builder_.GetInsertBlock()->getParent();
        llvm::BasicBlock *preheaderBB = builder_.GetInsertBlock();
        llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(context_, "loop", theFunction);

        builder_.CreateBr(loopBB);
        builder_.SetInsertPoint(loopBB);

        llvm::PHINode *variable = builder_.CreatePHI(llvm::Type::getDoubleTy(context_), 2, forExpr->getVarName());
        variable->addIncoming(startV, preheaderBB);

        llvm::Value *oldVal = namedValues_[forExpr->getVarName()];
        namedValues_[forExpr->getVarName()] = variable;

        if (!codegen(*const_cast<ExprAST *>(forExpr->getBody()))) {
            return nullptr;
        }

        llvm::Value *stepV = nullptr;
        if (forExpr->getStep()) {
            stepV = codegen(*const_cast<ExprAST *>(forExpr->getStep()));
            if (!stepV) return nullptr;
        } else {
            stepV = llvm::ConstantFP::get(context_, llvm::APFloat(1.0));
        }

        llvm::Value *nextVar = builder_.CreateFAdd(variable, stepV, "nextvar");

        llvm::Value *endCond = codegen(*const_cast<ExprAST *>(forExpr->getEnd()));
        if (!endCond) return nullptr;

        endCond = builder_.CreateFCmpONE(endCond, llvm::ConstantFP::get(context_, llvm::APFloat(0.0)), "loopcond");

        llvm::BasicBlock *loopEndBB = builder_.GetInsertBlock();
        llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(context_, "afterloop", theFunction);

        builder_.CreateCondBr(endCond, loopBB, afterBB);
        builder_.SetInsertPoint(afterBB);

        variable->addIncoming(nextVar, loopEndBB);

        if (oldVal) {
            namedValues_[forExpr->getVarName()] = oldVal;
        } else {
            namedValues_.erase(forExpr->getVarName());
        }

        return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(context_));
    }

    return nullptr;
}

llvm::Function *IRGenerator::codegen(PrototypeAST &proto) {
    std::vector<llvm::Type *> doubles(proto.getArgCount(), llvm::Type::getDoubleTy(context_));
    llvm::FunctionType *ft = llvm::FunctionType::get(llvm::Type::getDoubleTy(context_), doubles, false);
    llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, proto.getName(), module_.get());

    unsigned idx = 0;
    for (auto &arg : f->args()) {
        arg.setName(proto.getArgs()[idx++]);
    }

    return f;
}

llvm::Function *IRGenerator::codegen(FunctionAST &func) {
    functionProtos_[func.getName()] = func.getProto()->clone();
    llvm::Function *theFunction = getFunction(func.getName());
    if (!theFunction) return nullptr;

    if (!theFunction->empty()) {
        if (diag_) diag_->error(func.getLocation(), "Function cannot be redefined: '" + func.getName() + "'");
        return nullptr;
    }

    llvm::BasicBlock *bb = llvm::BasicBlock::Create(context_, "entry", theFunction);
    builder_.SetInsertPoint(bb);

    namedValues_.clear();
    for (auto &arg : theFunction->args()) {
        namedValues_[std::string(arg.getName())] = &arg;
    }

    if (llvm::Value *retVal = codegen(*func.getBody())) {
        builder_.CreateRet(retVal);
        llvm::verifyFunction(*theFunction);
        return theFunction;
    }

    theFunction->eraseFromParent();
    return nullptr;
}

void IRGenerator::dumpIR(std::ostream &os) {
    llvm::raw_os_ostream rawOs(os);
    module_->print(rawOs, nullptr);
}

std::string IRGenerator::getIRString() {
    std::ostringstream ss;
    dumpIR(ss);
    return ss.str();
}

#else // !KALEIDOSCOPE_HAS_LLVM (Standalone portable IR Generator)

IRGenerator::IRGenerator(const std::string &moduleName, DiagnosticEngine *diag)
    : moduleName_(moduleName), diag_(diag) {}

IRGenerator::~IRGenerator() = default;

void IRGenerator::resetModule(const std::string &moduleName) {
    moduleName_ = moduleName;
    textIRBuffer_.clear();
    functionProtos_.clear();
}

static int gTempReg = 1;

static std::string emitExprIR(ExprAST &expr, std::ostream &os) {
    if (auto *num = dynamic_cast<NumberExprAST *>(&expr)) {
        std::ostringstream ss;
        ss << num->getValue();
        return ss.str();
    }
    if (auto *var = dynamic_cast<VariableExprAST *>(&expr)) {
        return "%" + var->getName();
    }
    if (auto *unary = dynamic_cast<UnaryExprAST *>(&expr)) {
        std::string opReg = emitExprIR(*const_cast<ExprAST *>(unary->getOperand()), os);
        if (unary->getOpcode() == '-') {
            std::string res = "%" + std::to_string(gTempReg++);
            os << "  " << res << " = fneg double " << opReg << "\n";
            return res;
        }
        return opReg;
    }
    if (auto *bin = dynamic_cast<BinaryExprAST *>(&expr)) {
        std::string lhs = emitExprIR(*const_cast<ExprAST *>(bin->getLHS()), os);
        std::string rhs = emitExprIR(*const_cast<ExprAST *>(bin->getRHS()), os);
        std::string res = "%" + std::to_string(gTempReg++);
        const char *opName = "fadd";
        switch (bin->getOp()) {
            case '+': opName = "fadd"; break;
            case '-': opName = "fsub"; break;
            case '*': opName = "fmul"; break;
            case '/': opName = "fdiv"; break;
            case '<':
                os << "  " << res << " = fcmp ult double " << lhs << ", " << rhs << "\n";
                {
                    std::string boolRes = "%" + std::to_string(gTempReg++);
                    os << "  " << boolRes << " = uitofp i1 " << res << " to double\n";
                    return boolRes;
                }
            case '>':
                os << "  " << res << " = fcmp ugt double " << lhs << ", " << rhs << "\n";
                {
                    std::string boolRes = "%" + std::to_string(gTempReg++);
                    os << "  " << boolRes << " = uitofp i1 " << res << " to double\n";
                    return boolRes;
                }
        }
        os << "  " << res << " = " << opName << " double " << lhs << ", " << rhs << "\n";
        return res;
    }
    if (auto *call = dynamic_cast<CallExprAST *>(&expr)) {
        std::vector<std::string> argRegs;
        for (auto &arg : const_cast<std::vector<std::unique_ptr<ExprAST>> &>(call->getArgs())) {
            argRegs.push_back(emitExprIR(*arg, os));
        }
        std::string res = "%" + std::to_string(gTempReg++);
        os << "  " << res << " = call double @" << call->getCallee() << "(";
        for (size_t i = 0; i < argRegs.size(); ++i) {
            if (i > 0) os << ", ";
            os << "double " << argRegs[i];
        }
        os << ")\n";
        return res;
    }
    if (auto *ifExpr = dynamic_cast<IfExprAST *>(&expr)) {
        std::string cond = emitExprIR(*const_cast<ExprAST *>(ifExpr->getCond()), os);
        int ifId = gTempReg++;
        os << "  br i1 " << cond << ", label %then." << ifId << ", label %else." << ifId << "\n\n";
        os << "then." << ifId << ":\n";
        std::string thenVal = emitExprIR(*const_cast<ExprAST *>(ifExpr->getThen()), os);
        os << "  br label %merge." << ifId << "\n\n";
        os << "else." << ifId << ":\n";
        std::string elseVal = emitExprIR(*const_cast<ExprAST *>(ifExpr->getElse()), os);
        os << "  br label %merge." << ifId << "\n\n";
        os << "merge." << ifId << ":\n";
        std::string res = "%" + std::to_string(gTempReg++);
        os << "  " << res << " = phi double [ " << thenVal << ", %then." << ifId
           << " ], [ " << elseVal << ", %else." << ifId << " ]\n";
        return res;
    }
    return "0.0";
}

bool IRGenerator::codegen(PrototypeAST &proto, std::ostream &os) {
    os << "declare double @" << proto.getName() << "(";
    for (size_t i = 0; i < proto.getArgCount(); ++i) {
        if (i > 0) os << ", ";
        os << "double %" << proto.getArgs()[i];
    }
    os << ")\n\n";
    return true;
}

bool IRGenerator::codegen(FunctionAST &func, std::ostream &os) {
    const auto *proto = func.getProto();
    os << "define double @" << proto->getName() << "(";
    for (size_t i = 0; i < proto->getArgCount(); ++i) {
        if (i > 0) os << ", ";
        os << "double %" << proto->getArgs()[i];
    }
    os << ") {\n";
    os << "entry:\n";

    gTempReg = 1;
    std::string retVal = emitExprIR(*func.getBody(), os);
    os << "  ret double " << retVal << "\n";
    os << "}\n\n";
    return true;
}

bool IRGenerator::codegen(ExprAST &expr, std::ostream &os) {
    emitExprIR(expr, os);
    return true;
}

void IRGenerator::dumpIR(std::ostream &os) {
    os << textIRBuffer_;
}

std::string IRGenerator::getIRString() {
    return textIRBuffer_;
}

#endif

} // namespace kaleidoscope
