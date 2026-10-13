#ifndef KALEIDOSCOPE_AST_VISITOR_HPP
#define KALEIDOSCOPE_AST_VISITOR_HPP

#include "AST.hpp"
#include <iostream>

namespace kaleidoscope {

class NumberExprAST;
class VariableExprAST;
class UnaryExprAST;
class BinaryExprAST;
class CallExprAST;
class IfExprAST;
class ForExprAST;

/// Generic Visitor interface for traversing AST nodes.
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(NumberExprAST &node) = 0;
    virtual void visit(VariableExprAST &node) = 0;
    virtual void visit(UnaryExprAST &node) = 0;
    virtual void visit(BinaryExprAST &node) = 0;
    virtual void visit(CallExprAST &node) = 0;
    virtual void visit(IfExprAST &node) = 0;
    virtual void visit(ForExprAST &node) = 0;
};

/// Pretty-printer AST Dumper that prints an indented tree representation.
class ASTDumper : public ASTVisitor {
public:
    explicit ASTDumper(std::ostream &os = std::cout);

    void dump(ExprAST &expr);
    void dump(const PrototypeAST &proto);
    void dump(FunctionAST &func);

    void visit(NumberExprAST &node) override;
    void visit(VariableExprAST &node) override;
    void visit(UnaryExprAST &node) override;
    void visit(BinaryExprAST &node) override;
    void visit(CallExprAST &node) override;
    void visit(IfExprAST &node) override;
    void visit(ForExprAST &node) override;


private:
    void indent();
    std::ostream &os_;
    int indentLevel_{0};
};

} // namespace kaleidoscope

#endif // KALEIDOSCOPE_AST_VISITOR_HPP
