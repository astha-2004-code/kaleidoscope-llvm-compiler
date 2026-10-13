#include "kaleidoscope/Lexer.hpp"
#include <sstream>
#include <cctype>

namespace kaleidoscope {

Lexer::Lexer(std::istream &stream, DiagnosticEngine *diag)
    : stream_(&stream), diag_(diag) {
    advance(); // Initialize currentChar_
}

Lexer::Lexer(const std::string &source, DiagnosticEngine *diag)
    : ownedStream_(std::make_unique<std::istringstream>(source)),
      stream_(ownedStream_.get()),
      diag_(diag) {
    advance(); // Initialize currentChar_
}

char Lexer::getChar() {
    int c = stream_->get();
    if (c == EOF) {
        return '\0';
    }
    return static_cast<char>(c);
}

char Lexer::peekChar() {
    int c = stream_->peek();
    if (c == EOF) {
        return '\0';
    }
    return static_cast<char>(c);
}

void Lexer::advance() {
    if (currentChar_ == '\n') {
        ++line_;
        column_ = 0;
    }
    currentChar_ = getChar();
    if (currentChar_ != '\0') {
        ++column_;
    }
}

void Lexer::skipWhitespaceAndComments() {
    while (true) {
        // Skip whitespace
        while (currentChar_ != '\0' && (currentChar_ == ' ' || currentChar_ == '\t' ||
                                        currentChar_ == '\r' || currentChar_ == '\n')) {
            advance();
        }

        // Check for comment starting with '#'
        if (currentChar_ == '#') {
            while (currentChar_ != '\0' && currentChar_ != '\n') {
                advance();
            }
        } else {
            break;
        }
    }
}

Token Lexer::lexIdentifierOrKeyword() {
    SourceLocation startLoc(line_, column_);
    std::string ident;

    while (std::isalnum(static_cast<unsigned char>(currentChar_)) || currentChar_ == '_') {
        ident.push_back(currentChar_);
        advance();
    }

    if (ident == "def")    return Token(TokenKind::Def, startLoc, ident);
    if (ident == "extern") return Token(TokenKind::Extern, startLoc, ident);
    if (ident == "if")     return Token(TokenKind::If, startLoc, ident);
    if (ident == "then")   return Token(TokenKind::Then, startLoc, ident);
    if (ident == "else")   return Token(TokenKind::Else, startLoc, ident);
    if (ident == "for")    return Token(TokenKind::For, startLoc, ident);
    if (ident == "in")     return Token(TokenKind::In, startLoc, ident);

    return Token(TokenKind::Identifier, startLoc, ident);
}

Token Lexer::lexNumber() {
    SourceLocation startLoc(line_, column_);
    std::string numStr;
    bool hasDot = false;

    while (std::isdigit(static_cast<unsigned char>(currentChar_)) || (!hasDot && currentChar_ == '.')) {
        if (currentChar_ == '.') {
            hasDot = true;
        }
        numStr.push_back(currentChar_);
        advance();
    }

    double value = 0.0;
    try {
        value = std::stod(numStr);
    } catch (...) {
        if (diag_) {
            diag_->error(startLoc, "Invalid floating-point literal: '" + numStr + "'");
        }
    }

    return Token(TokenKind::Number, startLoc, numStr, value);
}

Token Lexer::nextToken() {
    if (hasPeeked_) {
        hasPeeked_ = false;
        currentToken_ = peekedToken_;
        return currentToken_;
    }

    skipWhitespaceAndComments();

    if (currentChar_ == '\0') {
        currentToken_ = Token(TokenKind::EndOfFile, SourceLocation(line_, column_), "");
        return currentToken_;
    }

    SourceLocation loc(line_, column_);

    // Identifier or Keyword
    if (std::isalpha(static_cast<unsigned char>(currentChar_)) || currentChar_ == '_') {
        currentToken_ = lexIdentifierOrKeyword();
        return currentToken_;
    }

    // Number literal: digits or leading decimal dot
    if (std::isdigit(static_cast<unsigned char>(currentChar_)) ||
        (currentChar_ == '.' && std::isdigit(static_cast<unsigned char>(peekChar())))) {
        currentToken_ = lexNumber();
        return currentToken_;
    }

    // Punctuation and Single-character operators
    char op = currentChar_;
    advance();

    switch (op) {
        case ';': currentToken_ = Token(TokenKind::Semicolon, loc, ";"); break;
        case '(': currentToken_ = Token(TokenKind::LeftParen, loc, "("); break;
        case ')': currentToken_ = Token(TokenKind::RightParen, loc, ")"); break;
        case ',': currentToken_ = Token(TokenKind::Comma, loc, ","); break;
        case '+': currentToken_ = Token(TokenKind::Plus, loc, "+"); break;
        case '-': currentToken_ = Token(TokenKind::Minus, loc, "-"); break;
        case '*': currentToken_ = Token(TokenKind::Star, loc, "*"); break;
        case '/': currentToken_ = Token(TokenKind::Slash, loc, "/"); break;
        case '<': currentToken_ = Token(TokenKind::LessThan, loc, "<"); break;
        case '>': currentToken_ = Token(TokenKind::GreaterThan, loc, ">"); break;
        case '=': currentToken_ = Token(TokenKind::Equal, loc, "="); break;
        default:
            currentToken_ = Token(TokenKind::Unknown, loc, std::string(1, op));
            if (diag_) {
                diag_->error(loc, std::string("Unexpected character: '") + op + "'");
            }
            break;
    }

    return currentToken_;
}

const Token &Lexer::peekToken() {
    if (!hasPeeked_) {
        Token prev = currentToken_;
        peekedToken_ = nextToken();
        currentToken_ = prev;
        hasPeeked_ = true;
    }
    return peekedToken_;
}

} // namespace kaleidoscope
