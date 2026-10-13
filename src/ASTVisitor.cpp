#include "kaleidoscope/ASTVisitor.hpp"

namespace kaleidoscope {

ASTDumper::ASTDumper(std::ostream &os) : os_(os) {}

void ASTDumper::indent() {
    for (int i = 0; i < indentLevel_; ++i) {
        os_ << "  ";
    }
}

void ASTDumper::dump(ExprAST &expr) {
    expr.accept(*this);
    os_ << "\n";
}

void ASTDumper::dump(const PrototypeAST &proto) {
    indent();
    os_ << "(Prototype " << proto.getName() << " [";
    for (size_t i = 0; i < proto.getArgs().size(); ++i) {
        if (i > 0) os_ << ", ";
        os_ << proto.getArgs()[i];
    }
    os_ << "])\n";
}

void ASTDumper::dump(FunctionAST &func) {
    indent();
    os_ << "(Function\n";
    ++indentLevel_;
    dump(*func.getProto());
    if (func.getBody()) {
        indent();
        os_ << "Body:\n";
        ++indentLevel_;
        indent();
        func.getBody()->accept(*this);
        os_ << "\n";
        --indentLevel_;
    }
    --indentLevel_;
    indent();
    os_ << ")\n";
}

void ASTDumper::visit(NumberExprAST &node) {
    os_ << "(Number " << node.getValue() << ")";
}

void ASTDumper::visit(VariableExprAST &node) {
    os_ << "(Variable \"" << node.getName() << "\")";
}

void ASTDumper::visit(UnaryExprAST &node) {
    os_ << "(Unary '" << node.getOpcode() << "' ";
    if (node.getOperand()) {
        node.getOperand()->accept(*this);
    }
    os_ << ")";
}


void ASTDumper::visit(BinaryExprAST &node) {
    os_ << "(Binary '" << node.getOp() << "'\n";
    ++indentLevel_;
    indent();
    os_ << "LHS: ";
    if (node.getLHS()) {
        node.getLHS()->accept(*this);
    }
    os_ << "\n";
    indent();
    os_ << "RHS: ";
    if (node.getRHS()) {
        node.getRHS()->accept(*this);
    }
    --indentLevel_;
    os_ << ")";
}

void ASTDumper::visit(CallExprAST &node) {
    os_ << "(Call \"" << node.getCallee() << "\" [";
    for (size_t i = 0; i < node.getArgs().size(); ++i) {
        if (i > 0) os_ << ", ";
        if (node.getArgs()[i]) {
            node.getArgs()[i]->accept(*this);
        }
    }
    os_ << "])";
}

void ASTDumper::visit(IfExprAST &node) {
    os_ << "(If\n";
    ++indentLevel_;
    indent(); os_ << "Cond: ";
    if (node.getCond()) node.getCond()->accept(*this);
    os_ << "\n";
    indent(); os_ << "Then: ";
    if (node.getThen()) node.getThen()->accept(*this);
    os_ << "\n";
    indent(); os_ << "Else: ";
    if (node.getElse()) node.getElse()->accept(*this);
    --indentLevel_;
    os_ << ")";
}

void ASTDumper::visit(ForExprAST &node) {
    os_ << "(For \"" << node.getVarName() << "\"\n";
    ++indentLevel_;
    indent(); os_ << "Start: ";
    if (node.getStart()) node.getStart()->accept(*this);
    os_ << "\n";
    indent(); os_ << "End: ";
    if (node.getEnd()) node.getEnd()->accept(*this);
    os_ << "\n";
    if (node.getStep()) {
        indent(); os_ << "Step: ";
        node.getStep()->accept(*this);
        os_ << "\n";
    }
    indent(); os_ << "Body: ";
    if (node.getBody()) node.getBody()->accept(*this);
    --indentLevel_;
    os_ << ")";
}

} // namespace kaleidoscope
