#ifndef KALEIDOSCOPE_PARSER_HPP
#define KALEIDOSCOPE_PARSER_HPP

#include "Common.hpp"
#include "Token.hpp"
#include "Lexer.hpp"
#include "AST.hpp"
#include "Diagnostic.hpp"
#include <memory>
#include <vector>

namespace kaleidoscope {

class Parser {
public:
    explicit Parser(Lexer &lexer, DiagnosticEngine *diag = nullptr);

    /// Top-level production parsing
    std::unique_ptr<FunctionAST> parseDefinition();
    std::unique_ptr<PrototypeAST> parseExtern();
    std::unique_ptr<FunctionAST> parseTopLevelExpr();

    /// Expression parsing
    std::unique_ptr<ExprAST> parseExpression();
    std::unique_ptr<ExprAST> parseUnary();
    std::unique_ptr<ExprAST> parsePrimary();
    std::unique_ptr<ExprAST> parseNumberExpr();

    std::unique_ptr<ExprAST> parseParenExpr();
    std::unique_ptr<ExprAST> parseIdentifierExpr();
    std::unique_ptr<ExprAST> parseIfExpr();
    std::unique_ptr<ExprAST> parseForExpr();
    std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs);

    /// Prototype parsing: id '(' id* ')'
    std::unique_ptr<PrototypeAST> parsePrototype();

    /// Token inspection and movement
    const Token &currentToken() const { return currentToken_; }
    void getNextToken();

    /// Error synchronization
    void synchronize();

private:
    std::unique_ptr<ExprAST> error(const std::string &message);
    std::unique_ptr<PrototypeAST> errorProto(const std::string &message);
    std::unique_ptr<FunctionAST> errorFunc(const std::string &message);

    Lexer &lexer_;
    DiagnosticEngine *diag_{nullptr};
    Token currentToken_;
};

} // namespace kaleidoscope

#endif // KALEIDOSCOPE_PARSER_HPP
