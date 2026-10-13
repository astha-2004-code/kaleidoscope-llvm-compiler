#ifndef KALEIDOSCOPE_LEXER_HPP
#define KALEIDOSCOPE_LEXER_HPP

#include "Common.hpp"
#include "Token.hpp"
#include "Diagnostic.hpp"
#include <istream>
#include <string>
#include <memory>

namespace kaleidoscope {

class Lexer {
public:
    explicit Lexer(std::istream &stream, DiagnosticEngine *diag = nullptr);
    explicit Lexer(const std::string &source, DiagnosticEngine *diag = nullptr);

    Token nextToken();
    const Token &peekToken();
    const Token &currentToken() const { return currentToken_; }
    SourceLocation getCurrentLocation() const { return SourceLocation(line_, column_); }

private:
    char getChar();
    char peekChar();
    void advance();
    void skipWhitespaceAndComments();

    Token lexIdentifierOrKeyword();
    Token lexNumber();

    std::istream *stream_{nullptr};
    std::unique_ptr<std::istream> ownedStream_;
    DiagnosticEngine *diag_{nullptr};

    char currentChar_{' '};
    uint32_t line_{1};
    uint32_t column_{0};

    Token currentToken_;
    Token peekedToken_;
    bool hasPeeked_{false};
};

} // namespace kaleidoscope

#endif // KALEIDOSCOPE_LEXER_HPP
