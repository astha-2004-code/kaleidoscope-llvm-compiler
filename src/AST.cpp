#include "kaleidoscope/AST.hpp"
#include "kaleidoscope/ASTVisitor.hpp"

namespace kaleidoscope {

void NumberExprAST::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::unique_ptr<ExprAST> NumberExprAST::clone() const {
    return std::make_unique<NumberExprAST>(val_, getLocation());
}

void VariableExprAST::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::unique_ptr<ExprAST> VariableExprAST::clone() const {
    return std::make_unique<VariableExprAST>(name_, getLocation());
}

void UnaryExprAST::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::unique_ptr<ExprAST> UnaryExprAST::clone() const {
    return std::make_unique<UnaryExprAST>(opcode_, operand_ ? operand_->clone() : nullptr, getLocation());
}


void BinaryExprAST::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::unique_ptr<ExprAST> BinaryExprAST::clone() const {
    return std::make_unique<BinaryExprAST>(op_,
                                           lhs_ ? lhs_->clone() : nullptr,
                                           rhs_ ? rhs_->clone() : nullptr,
                                           getLocation());
}

void CallExprAST::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::unique_ptr<ExprAST> CallExprAST::clone() const {
    std::vector<std::unique_ptr<ExprAST>> args;
    args.reserve(args_.size());
    for (const auto &arg : args_) {
        args.push_back(arg ? arg->clone() : nullptr);
    }
    return std::make_unique<CallExprAST>(callee_, std::move(args), getLocation());
}

void IfExprAST::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::unique_ptr<ExprAST> IfExprAST::clone() const {
    return std::make_unique<IfExprAST>(cond_ ? cond_->clone() : nullptr,
                                       thenExpr_ ? thenExpr_->clone() : nullptr,
                                       elseExpr_ ? elseExpr_->clone() : nullptr,
                                       getLocation());
}

void ForExprAST::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::unique_ptr<ExprAST> ForExprAST::clone() const {
    return std::make_unique<ForExprAST>(varName_,
                                        start_ ? start_->clone() : nullptr,
                                        end_ ? end_->clone() : nullptr,
                                        step_ ? step_->clone() : nullptr,
                                        body_ ? body_->clone() : nullptr,
                                        getLocation());
}

} // namespace kaleidoscope
