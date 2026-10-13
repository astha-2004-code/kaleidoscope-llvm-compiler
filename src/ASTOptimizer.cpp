#include "kaleidoscope/ASTOptimizer.hpp"
#include <cmath>

namespace kaleidoscope {

bool ASTConstantFolder::isNumber(const ExprAST *expr, double &val) {
    if (const auto *num = dynamic_cast<const NumberExprAST *>(expr)) {
        val = num->getValue();
        return true;
    }
    return false;
}

bool ASTConstantFolder::isZero(const ExprAST *expr) {
    double v = 0.0;
    return isNumber(expr, v) && std::fabs(v) < 1e-12;
}

bool ASTConstantFolder::isOne(const ExprAST *expr) {
    double v = 0.0;
    return isNumber(expr, v) && std::fabs(v - 1.0) < 1e-12;
}

std::unique_ptr<ExprAST> ASTConstantFolder::foldBinary(std::unique_ptr<BinaryExprAST> bin) {
    SourceLocation loc = bin->getLocation();
    char op = bin->getOp();

    auto lhs = optimize(bin->releaseLHS());
    auto rhs = optimize(bin->releaseRHS());

    double lVal = 0.0;
    double rVal = 0.0;
    bool lIsNum = isNumber(lhs.get(), lVal);
    bool rIsNum = isNumber(rhs.get(), rVal);

    // Constant folding if both operands are numeric constants
    if (lIsNum && rIsNum) {
        switch (op) {
            case '+': return std::make_unique<NumberExprAST>(lVal + rVal, loc);
            case '-': return std::make_unique<NumberExprAST>(lVal - rVal, loc);
            case '*': return std::make_unique<NumberExprAST>(lVal * rVal, loc);
            case '/':
                if (std::fabs(rVal) > 1e-12) {
                    return std::make_unique<NumberExprAST>(lVal / rVal, loc);
                }
                break;
            case '<': return std::make_unique<NumberExprAST>(lVal < rVal ? 1.0 : 0.0, loc);
            case '>': return std::make_unique<NumberExprAST>(lVal > rVal ? 1.0 : 0.0, loc);
            default: break;
        }
    }

    // Algebraic identity simplifications
    if (op == '+') {
        if (isZero(lhs.get())) return rhs; // 0 + x -> x
        if (isZero(rhs.get())) return lhs; // x + 0 -> x
    } else if (op == '-') {
        if (isZero(rhs.get())) return lhs; // x - 0 -> x
    } else if (op == '*') {
        if (isZero(lhs.get()) || isZero(rhs.get())) {
            return std::make_unique<NumberExprAST>(0.0, loc); // x * 0 or 0 * x -> 0
        }
        if (isOne(lhs.get())) return rhs; // 1 * x -> x
        if (isOne(rhs.get())) return lhs; // x * 1 -> x
    } else if (op == '/') {
        if (isOne(rhs.get())) return lhs; // x / 1 -> x
    }

    return std::make_unique<BinaryExprAST>(op, std::move(lhs), std::move(rhs), loc);
}

std::unique_ptr<ExprAST> ASTConstantFolder::foldUnary(std::unique_ptr<UnaryExprAST> unary) {
    SourceLocation loc = unary->getLocation();
    char op = unary->getOpcode();

    auto operand = optimize(std::unique_ptr<ExprAST>(unary->getOperand()->clone()));

    double val = 0.0;
    if (isNumber(operand.get(), val)) {
        if (op == '-') return std::make_unique<NumberExprAST>(-val, loc);
        if (op == '+') return std::make_unique<NumberExprAST>(val, loc);
    }

    return std::make_unique<UnaryExprAST>(op, std::move(operand), loc);
}

std::unique_ptr<ExprAST> ASTConstantFolder::foldIf(std::unique_ptr<IfExprAST> ifExpr) {
    SourceLocation loc = ifExpr->getLocation();

    auto cond = optimize(std::unique_ptr<ExprAST>(ifExpr->getCond()->clone()));
    auto thenExpr = optimize(std::unique_ptr<ExprAST>(ifExpr->getThen()->clone()));
    auto elseExpr = optimize(std::unique_ptr<ExprAST>(ifExpr->getElse()->clone()));

    double condVal = 0.0;
    if (isNumber(cond.get(), condVal)) {
        // Condition is known at compile time: eliminate dead branch
        if (std::fabs(condVal) > 1e-12) {
            return thenExpr;
        } else {
            return elseExpr;
        }
    }

    return std::make_unique<IfExprAST>(std::move(cond), std::move(thenExpr), std::move(elseExpr), loc);
}

std::unique_ptr<ExprAST> ASTConstantFolder::foldFor(std::unique_ptr<ForExprAST> forExpr) {
    SourceLocation loc = forExpr->getLocation();
    std::string var = forExpr->getVarName();

    auto start = optimize(std::unique_ptr<ExprAST>(forExpr->getStart()->clone()));
    auto end = optimize(std::unique_ptr<ExprAST>(forExpr->getEnd()->clone()));
    std::unique_ptr<ExprAST> step = nullptr;
    if (forExpr->getStep()) {
        step = optimize(std::unique_ptr<ExprAST>(forExpr->getStep()->clone()));
    }
    auto body = optimize(std::unique_ptr<ExprAST>(forExpr->getBody()->clone()));

    return std::make_unique<ForExprAST>(var, std::move(start), std::move(end), std::move(step), std::move(body), loc);
}

std::unique_ptr<ExprAST> ASTConstantFolder::foldCall(std::unique_ptr<CallExprAST> callExpr) {
    SourceLocation loc = callExpr->getLocation();
    std::string callee = callExpr->getCallee();
    std::vector<std::unique_ptr<ExprAST>> optArgs;
    for (auto &arg : callExpr->getArgs()) {
        optArgs.push_back(optimize(std::move(arg)));
    }
    return std::make_unique<CallExprAST>(callee, std::move(optArgs), loc);
}

std::unique_ptr<ExprAST> ASTConstantFolder::optimize(std::unique_ptr<ExprAST> expr) {
    if (!expr) return nullptr;

    if (auto *bin = dynamic_cast<BinaryExprAST *>(expr.get())) {
        expr.release();
        return foldBinary(std::unique_ptr<BinaryExprAST>(bin));
    }
    if (auto *unary = dynamic_cast<UnaryExprAST *>(expr.get())) {
        expr.release();
        return foldUnary(std::unique_ptr<UnaryExprAST>(unary));
    }
    if (auto *ifExpr = dynamic_cast<IfExprAST *>(expr.get())) {
        expr.release();
        return foldIf(std::unique_ptr<IfExprAST>(ifExpr));
    }
    if (auto *forExpr = dynamic_cast<ForExprAST *>(expr.get())) {
        expr.release();
        return foldFor(std::unique_ptr<ForExprAST>(forExpr));
    }
    if (auto *callExpr = dynamic_cast<CallExprAST *>(expr.get())) {
        expr.release();
        return foldCall(std::unique_ptr<CallExprAST>(callExpr));
    }

    return expr;
}

void ASTConstantFolder::optimize(FunctionAST &func) {
    if (func.getBody()) {
        auto body = func.releaseBody();
        func.setBody(optimize(std::move(body)));
    }
}

} // namespace kaleidoscope
