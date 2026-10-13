#include "kaleidoscope/Parser.hpp"

namespace kaleidoscope {

Parser::Parser(Lexer &lexer, DiagnosticEngine *diag)
    : lexer_(lexer), diag_(diag) {
    getNextToken(); // Prime the parser with the first token
}

void Parser::getNextToken() {
    currentToken_ = lexer_.nextToken();
}

std::unique_ptr<ExprAST> Parser::error(const std::string &message) {
    if (diag_) {
        diag_->error(currentToken_.location, message);
    }
    return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::errorProto(const std::string &message) {
    if (diag_) {
        diag_->error(currentToken_.location, message);
    }
    return nullptr;
}

std::unique_ptr<FunctionAST> Parser::errorFunc(const std::string &message) {
    if (diag_) {
        diag_->error(currentToken_.location, message);
    }
    return nullptr;
}

void Parser::synchronize() {
    while (currentToken_.isNot(TokenKind::EndOfFile)) {
        if (currentToken_.is(TokenKind::Semicolon)) {
            getNextToken();
            return;
        }
        if (currentToken_.is(TokenKind::Def) || currentToken_.is(TokenKind::Extern)) {
            return;
        }
        getNextToken();
    }
}

// numberexpr ::= number
std::unique_ptr<ExprAST> Parser::parseNumberExpr() {
    auto result = std::make_unique<NumberExprAST>(currentToken_.numberValue, currentToken_.location);
    getNextToken(); // consume number
    return result;
}

// parenexpr ::= '(' expression ')'
std::unique_ptr<ExprAST> Parser::parseParenExpr() {
    getNextToken(); // consume '('
    auto expr = parseExpression();
    if (!expr) {
        return nullptr;
    }
    if (currentToken_.isNot(TokenKind::RightParen)) {
        return error("Expected ')' after parenthesized expression");
    }
    getNextToken(); // consume ')'
    return expr;
}

// identifierexpr
//   ::= identifier
//   ::= identifier '(' expression* ')'
std::unique_ptr<ExprAST> Parser::parseIdentifierExpr() {
    std::string idName = currentToken_.lexeme;
    SourceLocation loc = currentToken_.location;
    getNextToken(); // consume identifier

    // Variable reference
    if (currentToken_.isNot(TokenKind::LeftParen)) {
        return std::make_unique<VariableExprAST>(std::move(idName), loc);
    }

    // Function call
    getNextToken(); // consume '('
    std::vector<std::unique_ptr<ExprAST>> args;
    if (currentToken_.isNot(TokenKind::RightParen)) {
        while (true) {
            if (auto arg = parseExpression()) {
                args.push_back(std::move(arg));
            } else {
                return nullptr;
            }

            if (currentToken_.is(TokenKind::RightParen)) {
                break;
            }

            if (currentToken_.isNot(TokenKind::Comma)) {
                return error("Expected ')' or ',' in argument list");
            }
            getNextToken(); // consume ','
        }
    }

    getNextToken(); // consume ')'
    return std::make_unique<CallExprAST>(std::move(idName), std::move(args), loc);
}

// ifexpr ::= 'if' expression 'then' expression 'else' expression
std::unique_ptr<ExprAST> Parser::parseIfExpr() {
    SourceLocation loc = currentToken_.location;
    getNextToken(); // consume 'if'

    auto cond = parseExpression();
    if (!cond) return nullptr;

    if (currentToken_.isNot(TokenKind::Then)) {
        return error("Expected 'then' after if-condition");
    }
    getNextToken(); // consume 'then'

    auto thenExpr = parseExpression();
    if (!thenExpr) return nullptr;

    if (currentToken_.isNot(TokenKind::Else)) {
        return error("Expected 'else' in conditional expression");
    }
    getNextToken(); // consume 'else'

    auto elseExpr = parseExpression();
    if (!elseExpr) return nullptr;

    return std::make_unique<IfExprAST>(std::move(cond), std::move(thenExpr), std::move(elseExpr), loc);
}

// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expr
std::unique_ptr<ExprAST> Parser::parseForExpr() {
    SourceLocation loc = currentToken_.location;
    getNextToken(); // consume 'for'

    if (currentToken_.isNot(TokenKind::Identifier)) {
        return error("Expected variable identifier after 'for'");
    }
    std::string idName = currentToken_.lexeme;
    getNextToken(); // consume identifier

    if (currentToken_.isNot(TokenKind::Equal)) {
        return error("Expected '=' after for variable");
    }
    getNextToken(); // consume '='

    auto start = parseExpression();
    if (!start) return nullptr;

    if (currentToken_.isNot(TokenKind::Comma)) {
        return error("Expected ',' after for loop start value");
    }
    getNextToken(); // consume ','

    auto end = parseExpression();
    if (!end) return nullptr;

    std::unique_ptr<ExprAST> step;
    if (currentToken_.is(TokenKind::Comma)) {
        getNextToken(); // consume ','
        step = parseExpression();
        if (!step) return nullptr;
    }

    if (currentToken_.isNot(TokenKind::In)) {
        return error("Expected 'in' after for loop condition range");
    }
    getNextToken(); // consume 'in'

    auto body = parseExpression();
    if (!body) return nullptr;

    return std::make_unique<ForExprAST>(std::move(idName),
                                        std::move(start),
                                        std::move(end),
                                        std::move(step),
                                        std::move(body),
                                        loc);
}

// primary
//   ::= identifierexpr
//   ::= numberexpr
//   ::= parenexpr
//   ::= ifexpr
//   ::= forexpr
std::unique_ptr<ExprAST> Parser::parsePrimary() {
    switch (currentToken_.kind) {
        case TokenKind::Identifier: return parseIdentifierExpr();
        case TokenKind::Number:     return parseNumberExpr();
        case TokenKind::LeftParen:  return parseParenExpr();
        case TokenKind::If:         return parseIfExpr();
        case TokenKind::For:        return parseForExpr();
        case TokenKind::EndOfFile:  return nullptr;
        default:
            return error("Unexpected token '" + currentToken_.lexeme + "' when expecting expression");
    }
}

// unary
//   ::= primary
//   ::= ('+' | '-') unary
std::unique_ptr<ExprAST> Parser::parseUnary() {
    if (currentToken_.is(TokenKind::Plus) || currentToken_.is(TokenKind::Minus)) {
        char opcode = currentToken_.lexeme.empty() ? '+' : currentToken_.lexeme[0];
        SourceLocation loc = currentToken_.location;
        getNextToken();
        auto operand = parseUnary();
        if (!operand) return nullptr;
        return std::make_unique<UnaryExprAST>(opcode, std::move(operand), loc);
    }
    return parsePrimary();
}

// binoprhs ::= (op unary)*
std::unique_ptr<ExprAST> Parser::parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs) {
    while (true) {
        int tokPrec = currentToken_.getPrecedence();

        // If this operator binds less tightly than the current context, we are done
        if (tokPrec < exprPrec) {
            return lhs;
        }

        char binOp = currentToken_.lexeme.empty() ? '+' : currentToken_.lexeme[0];
        SourceLocation opLoc = currentToken_.location;
        getNextToken(); // consume binary operator

        // Parse unary expression after operator
        auto rhs = parseUnary();
        if (!rhs) {
            return nullptr;
        }

        // If next operator binds tighter, let next operator take RHS as its LHS
        int nextPrec = currentToken_.getPrecedence();
        if (tokPrec < nextPrec) {
            rhs = parseBinOpRHS(tokPrec + 1, std::move(rhs));
            if (!rhs) {
                return nullptr;
            }
        }

        // Merge LHS and RHS
        lhs = std::make_unique<BinaryExprAST>(binOp, std::move(lhs), std::move(rhs), opLoc);
    }
}

// expression ::= unary binoprhs
std::unique_ptr<ExprAST> Parser::parseExpression() {
    auto lhs = parseUnary();
    if (!lhs) {
        return nullptr;
    }
    return parseBinOpRHS(0, std::move(lhs));
}


// prototype ::= id '(' id* ')'
std::unique_ptr<PrototypeAST> Parser::parsePrototype() {
    if (currentToken_.isNot(TokenKind::Identifier)) {
        return errorProto("Expected function name in prototype");
    }

    std::string fnName = currentToken_.lexeme;
    SourceLocation loc = currentToken_.location;
    getNextToken(); // consume function name

    if (currentToken_.isNot(TokenKind::LeftParen)) {
        return errorProto("Expected '(' in prototype parameter list");
    }
    getNextToken(); // consume '('

    std::vector<std::string> argNames;
    while (currentToken_.is(TokenKind::Identifier)) {
        argNames.push_back(currentToken_.lexeme);
        getNextToken();
    }

    if (currentToken_.isNot(TokenKind::RightParen)) {
        return errorProto("Expected ')' after prototype parameter list");
    }
    getNextToken(); // consume ')'

    return std::make_unique<PrototypeAST>(std::move(fnName), std::move(argNames), loc);
}

// definition ::= 'def' prototype expression
std::unique_ptr<FunctionAST> Parser::parseDefinition() {
    getNextToken(); // consume 'def'
    auto proto = parsePrototype();
    if (!proto) {
        return nullptr;
    }

    auto body = parseExpression();
    if (!body) {
        return nullptr;
    }

    return std::make_unique<FunctionAST>(std::move(proto), std::move(body));
}

// external ::= 'extern' prototype
std::unique_ptr<PrototypeAST> Parser::parseExtern() {
    getNextToken(); // consume 'extern'
    return parsePrototype();
}

// toplevelexpr ::= expression
std::unique_ptr<FunctionAST> Parser::parseTopLevelExpr() {
    SourceLocation loc = currentToken_.location;
    if (auto expr = parseExpression()) {
        auto proto = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>{}, loc);
        return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));
    }
    return nullptr;
}

} // namespace kaleidoscope
