#include "kaleidoscope/Token.hpp"

namespace kaleidoscope {

const char *tokenKindToString(TokenKind kind) {
    switch (kind) {
        case TokenKind::EndOfFile:   return "EOF";
        case TokenKind::Identifier:  return "Identifier";
        case TokenKind::Number:      return "Number";
        case TokenKind::Def:         return "def";
        case TokenKind::Extern:      return "extern";
        case TokenKind::If:          return "if";
        case TokenKind::Then:        return "then";
        case TokenKind::Else:        return "else";
        case TokenKind::For:         return "for";
        case TokenKind::In:          return "in";
        case TokenKind::Semicolon:   return ";";
        case TokenKind::LeftParen:   return "(";
        case TokenKind::RightParen:  return ")";
        case TokenKind::Comma:       return ",";
        case TokenKind::Plus:        return "+";
        case TokenKind::Minus:       return "-";
        case TokenKind::Star:        return "*";
        case TokenKind::Slash:       return "/";
        case TokenKind::LessThan:    return "<";
        case TokenKind::GreaterThan: return ">";
        case TokenKind::Equal:       return "=";
        case TokenKind::Unknown:     return "Unknown";
    }
    return "Unknown";
}

int Token::getPrecedence() const {
    switch (kind) {
        case TokenKind::LessThan:
        case TokenKind::GreaterThan:
            return 10;
        case TokenKind::Plus:
        case TokenKind::Minus:
            return 20;
        case TokenKind::Star:
        case TokenKind::Slash:
            return 40;
        default:
            return -1;
    }
}

} // namespace kaleidoscope
