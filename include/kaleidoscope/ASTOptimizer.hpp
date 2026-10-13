#ifndef KALEIDOSCOPE_AST_OPTIMIZER_HPP
#define KALEIDOSCOPE_AST_OPTIMIZER_HPP

#include "AST.hpp"
#include <memory>

namespace kaleidoscope {

/// Performs AST-level constant folding and algebraic simplifications.
class ASTConstantFolder {
public:
    ASTConstantFolder() = default;

    /// Optimize an expression, returning the transformed AST root.
    std::unique_ptr<ExprAST> optimize(std::unique_ptr<ExprAST> expr);

    /// Optimize the body of a function.
    void optimize(FunctionAST &func);

private:
    std::unique_ptr<ExprAST> foldBinary(std::unique_ptr<BinaryExprAST> bin);
    std::unique_ptr<ExprAST> foldUnary(std::unique_ptr<UnaryExprAST> unary);
    std::unique_ptr<ExprAST> foldIf(std::unique_ptr<IfExprAST> ifExpr);
    std::unique_ptr<ExprAST> foldFor(std::unique_ptr<ForExprAST> forExpr);
    std::unique_ptr<ExprAST> foldCall(std::unique_ptr<CallExprAST> callExpr);

    static bool isNumber(const ExprAST *expr, double &val);
    static bool isZero(const ExprAST *expr);
    static bool isOne(const ExprAST *expr);
};

} // namespace kaleidoscope

#endif // KALEIDOSCOPE_AST_OPTIMIZER_HPP
