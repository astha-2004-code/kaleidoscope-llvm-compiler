#include "kaleidoscope/Lexer.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <cmath>

using namespace kaleidoscope;

static int gFailedTests = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "FAILED: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            ++gFailedTests; \
        } \
    } while(0)

void testKeywordsAndIdentifiers() {
    std::string source = "def extern if then else for in my_func foo123";
    Lexer lexer(source);

    Token tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Def), "Expected 'def'");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Extern), "Expected 'extern'");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::If), "Expected 'if'");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Then), "Expected 'then'");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Else), "Expected 'else'");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::For), "Expected 'for'");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::In), "Expected 'in'");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Identifier) && tok.lexeme == "my_func", "Expected identifier 'my_func'");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Identifier) && tok.lexeme == "foo123", "Expected identifier 'foo123'");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::EndOfFile), "Expected EOF");
}

void testNumbers() {
    std::string source = "42 3.14159 0.5 .75";
    Lexer lexer(source);

    Token tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Number) && std::fabs(tok.numberValue - 42.0) < 1e-6, "Expected 42");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Number) && std::fabs(tok.numberValue - 3.14159) < 1e-5, "Expected 3.14159");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Number) && std::fabs(tok.numberValue - 0.5) < 1e-6, "Expected 0.5");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Number) && std::fabs(tok.numberValue - 0.75) < 1e-6, "Expected 0.75");
}

void testOperatorsAndPunctuation() {
    std::string source = "+ - * / < > = ; ( ) ,";
    Lexer lexer(source);

    TokenKind expected[] = {
        TokenKind::Plus, TokenKind::Minus, TokenKind::Star, TokenKind::Slash,
        TokenKind::LessThan, TokenKind::GreaterThan, TokenKind::Equal,
        TokenKind::Semicolon, TokenKind::LeftParen, TokenKind::RightParen,
        TokenKind::Comma, TokenKind::EndOfFile
    };

    for (TokenKind k : expected) {
        Token tok = lexer.nextToken();
        TEST_ASSERT(tok.is(k), "Operator/punctuation mismatch");
    }
}

void testCommentsAndLocations() {
    std::string source =
        "# line 1 comment\n"
        "def foo(x) # line 2 comment\n"
        "    x + 1;";
    Lexer lexer(source);

    Token tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Def) && tok.location.line == 2 && tok.location.column == 1,
                "def location should be line 2, col 1");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Identifier) && tok.lexeme == "foo" && tok.location.line == 2 && tok.location.column == 5,
                "foo location should be line 2, col 5");

    tok = lexer.nextToken(); // (
    tok = lexer.nextToken(); // x
    tok = lexer.nextToken(); // )

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Identifier) && tok.lexeme == "x" && tok.location.line == 3 && tok.location.column == 5,
                "x location should be line 3, col 5");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Plus) && tok.location.line == 3 && tok.location.column == 7,
                "+ location should be line 3, col 7");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Number) && tok.numberValue == 1.0, "Expected 1");

    tok = lexer.nextToken();
    TEST_ASSERT(tok.is(TokenKind::Semicolon), "Expected semicolon");
}

int runLexerTests() {
    testKeywordsAndIdentifiers();
    testNumbers();
    testOperatorsAndPunctuation();
    testCommentsAndLocations();

    if (gFailedTests == 0) {
        std::cout << " [PASS] Lexer unit tests passed successfully.\n";
        return 0;
    } else {
        std::cerr << " [FAIL] Lexer unit tests failed with " << gFailedTests << " error(s).\n";
        return 1;
    }
}
