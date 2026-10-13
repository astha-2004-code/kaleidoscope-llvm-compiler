#ifndef KALEIDOSCOPE_AST_HPP
#define KALEIDOSCOPE_AST_HPP

#include "Common.hpp"
#include <string>
#include <vector>
#include <memory>

namespace kaleidoscope {

class ASTVisitor;

/// Base class for all expression nodes in Kaleidoscope.
class ExprAST {
public:
    explicit ExprAST(SourceLocation loc) : location_(loc) {}
    virtual ~ExprAST() = default;

    SourceLocation getLocation() const { return location_; }

    virtual void accept(ASTVisitor &visitor) = 0;
    virtual std::unique_ptr<ExprAST> clone() const = 0;

private:
    SourceLocation location_;
};

/// Numeric literal expression: e.g. 42.0, 3.14159
class NumberExprAST : public ExprAST {
public:
    NumberExprAST(double val, SourceLocation loc)
        : ExprAST(loc), val_(val) {}

    double getValue() const { return val_; }
    void setValue(double v) { val_ = v; }

    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ExprAST> clone() const override;

private:
    double val_;
};

/// Variable reference expression: e.g. 'x', 'my_var'
class VariableExprAST : public ExprAST {
public:
    VariableExprAST(std::string name, SourceLocation loc)
        : ExprAST(loc), name_(std::move(name)) {}

    const std::string &getName() const { return name_; }

    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ExprAST> clone() const override;

private:
    std::string name_;
};

/// Unary operator expression: e.g. '-x'
class UnaryExprAST : public ExprAST {
public:
    UnaryExprAST(char opcode, std::unique_ptr<ExprAST> operand, SourceLocation loc)
        : ExprAST(loc), opcode_(opcode), operand_(std::move(operand)) {}

    char getOpcode() const { return opcode_; }
    const ExprAST *getOperand() const { return operand_.get(); }
    ExprAST *getOperand() { return operand_.get(); }
    void setOperand(std::unique_ptr<ExprAST> op) { operand_ = std::move(op); }

    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ExprAST> clone() const override;

private:
    char opcode_;
    std::unique_ptr<ExprAST> operand_;
};


/// Binary operator expression: e.g. 'a + b', 'x < y'
class BinaryExprAST : public ExprAST {
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs, SourceLocation loc)
        : ExprAST(loc), op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    char getOp() const { return op_; }
    const ExprAST *getLHS() const { return lhs_.get(); }
    const ExprAST *getRHS() const { return rhs_.get(); }
    ExprAST *getLHS() { return lhs_.get(); }
    ExprAST *getRHS() { return rhs_.get(); }

    std::unique_ptr<ExprAST> releaseLHS() { return std::move(lhs_); }
    std::unique_ptr<ExprAST> releaseRHS() { return std::move(rhs_); }
    void setLHS(std::unique_ptr<ExprAST> lhs) { lhs_ = std::move(lhs); }
    void setRHS(std::unique_ptr<ExprAST> rhs) { rhs_ = std::move(rhs); }

    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ExprAST> clone() const override;

private:
    char op_;
    std::unique_ptr<ExprAST> lhs_;
    std::unique_ptr<ExprAST> rhs_;
};

/// Function call expression: e.g. 'foo(x, y + 1)'
class CallExprAST : public ExprAST {
public:
    CallExprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args, SourceLocation loc)
        : ExprAST(loc), callee_(std::move(callee)), args_(std::move(args)) {}

    const std::string &getCallee() const { return callee_; }
    const std::vector<std::unique_ptr<ExprAST>> &getArgs() const { return args_; }
    std::vector<std::unique_ptr<ExprAST>> &getArgs() { return args_; }

    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ExprAST> clone() const override;

private:
    std::string callee_;
    std::vector<std::unique_ptr<ExprAST>> args_;
};

/// Conditional expression: 'if <cond> then <then> else <else>'
class IfExprAST : public ExprAST {
public:
    IfExprAST(std::unique_ptr<ExprAST> cond,
              std::unique_ptr<ExprAST> thenExpr,
              std::unique_ptr<ExprAST> elseExpr,
              SourceLocation loc)
        : ExprAST(loc),
          cond_(std::move(cond)),
          thenExpr_(std::move(thenExpr)),
          elseExpr_(std::move(elseExpr)) {}

    const ExprAST *getCond() const { return cond_.get(); }
    const ExprAST *getThen() const { return thenExpr_.get(); }
    const ExprAST *getElse() const { return elseExpr_.get(); }
    ExprAST *getCond() { return cond_.get(); }
    ExprAST *getThen() { return thenExpr_.get(); }
    ExprAST *getElse() { return elseExpr_.get(); }

    void setCond(std::unique_ptr<ExprAST> c) { cond_ = std::move(c); }
    void setThen(std::unique_ptr<ExprAST> t) { thenExpr_ = std::move(t); }
    void setElse(std::unique_ptr<ExprAST> e) { elseExpr_ = std::move(e); }

    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ExprAST> clone() const override;

private:
    std::unique_ptr<ExprAST> cond_;
    std::unique_ptr<ExprAST> thenExpr_;
    std::unique_ptr<ExprAST> elseExpr_;
};

/// For-loop expression: 'for <var> = <start>, <end>, <step> in <body>'
class ForExprAST : public ExprAST {
public:
    ForExprAST(std::string varName,
               std::unique_ptr<ExprAST> start,
               std::unique_ptr<ExprAST> end,
               std::unique_ptr<ExprAST> step,
               std::unique_ptr<ExprAST> body,
               SourceLocation loc)
        : ExprAST(loc),
          varName_(std::move(varName)),
          start_(std::move(start)),
          end_(std::move(end)),
          step_(std::move(step)),
          body_(std::move(body)) {}

    const std::string &getVarName() const { return varName_; }
    const ExprAST *getStart() const { return start_.get(); }
    const ExprAST *getEnd() const { return end_.get(); }
    const ExprAST *getStep() const { return step_.get(); }
    const ExprAST *getBody() const { return body_.get(); }
    ExprAST *getStart() { return start_.get(); }
    ExprAST *getEnd() { return end_.get(); }
    ExprAST *getStep() { return step_.get(); }
    ExprAST *getBody() { return body_.get(); }

    void setBody(std::unique_ptr<ExprAST> b) { body_ = std::move(b); }

    void accept(ASTVisitor &visitor) override;
    std::unique_ptr<ExprAST> clone() const override;

private:
    std::string varName_;
    std::unique_ptr<ExprAST> start_;
    std::unique_ptr<ExprAST> end_;
    std::unique_ptr<ExprAST> step_;
    std::unique_ptr<ExprAST> body_;
};

/// Function signature (name and arguments)
class PrototypeAST {
public:
    PrototypeAST(std::string name, std::vector<std::string> args, SourceLocation loc)
        : name_(std::move(name)), args_(std::move(args)), location_(loc) {}

    const std::string &getName() const { return name_; }
    const std::vector<std::string> &getArgs() const { return args_; }
    SourceLocation getLocation() const { return location_; }
    size_t getArgCount() const { return args_.size(); }

    std::unique_ptr<PrototypeAST> clone() const {
        return std::make_unique<PrototypeAST>(name_, args_, location_);
    }

private:
    std::string name_;
    std::vector<std::string> args_;
    SourceLocation location_;
};

/// Complete function AST node: prototype + body expression
class FunctionAST {
public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
        : proto_(std::move(proto)), body_(std::move(body)) {}

    const PrototypeAST *getProto() const { return proto_.get(); }
    const ExprAST *getBody() const { return body_.get(); }
    PrototypeAST *getProto() { return proto_.get(); }
    ExprAST *getBody() { return body_.get(); }
    std::unique_ptr<ExprAST> releaseBody() { return std::move(body_); }
    void setBody(std::unique_ptr<ExprAST> b) { body_ = std::move(b); }

    const std::string &getName() const { return proto_->getName(); }
    SourceLocation getLocation() const { return proto_->getLocation(); }

    std::unique_ptr<FunctionAST> clone() const {
        return std::make_unique<FunctionAST>(proto_->clone(), body_ ? body_->clone() : nullptr);
    }

private:
    std::unique_ptr<PrototypeAST> proto_;
    std::unique_ptr<ExprAST> body_;
};

} // namespace kaleidoscope

#endif // KALEIDOSCOPE_AST_HPP
