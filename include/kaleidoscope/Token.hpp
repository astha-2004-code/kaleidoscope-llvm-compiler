#ifndef KALEIDOSCOPE_TOKEN_HPP
#define KALEIDOSCOPE_TOKEN_HPP

#include "Common.hpp"
#include <string>
#include <string_view>

namespace kaleidoscope {

enum class TokenKind {
    EndOfFile,
    Identifier,
    Number,

    // Keywords
    Def,
    Extern,
    If,
    Then,
    Else,
    For,
    In,

    // Punctuation & Delimiters
    Semicolon,
    LeftParen,
    RightParen,
    Comma,

    // Operators
    Plus,
    Minus,
    Star,
    Slash,
    LessThan,
    GreaterThan,
    Equal,

    Unknown
};

const char *tokenKindToString(TokenKind kind);

struct Token {
    TokenKind kind{TokenKind::EndOfFile};
    SourceLocation location{1, 1};
    std::string lexeme;
    double numberValue{0.0};

    Token() = default;
    Token(TokenKind k, SourceLocation loc, std::string lex, double val = 0.0)
        : kind(k), location(loc), lexeme(std::move(lex)), numberValue(val) {}

    bool is(TokenKind k) const { return kind == k; }
    bool isNot(TokenKind k) const { return kind != k; }

    /// Returns the binary operator precedence, or -1 if not a binary operator.
    int getPrecedence() const;
};

} // namespace kaleidoscope

#endif // KALEIDOSCOPE_TOKEN_HPP
