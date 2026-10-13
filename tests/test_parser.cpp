#include "kaleidoscope/Lexer.hpp"
#include "kaleidoscope/Parser.hpp"
#include "kaleidoscope/Diagnostic.hpp"
#include <cassert>
#include <iostream>

using namespace kaleidoscope;

static int gFailedParserTests = 0;

#define TEST_CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "PARSER TEST FAILED: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            ++gFailedParserTests; \
        } \
    } while(0)

void testParseBinaryPrecedence() {
    // 2 + 3 * 4 should parse as 2 + (3 * 4)
    std::string src = "2 + 3 * 4";
    Lexer lexer(src);
    Parser parser(lexer);

    auto expr = parser.parseExpression();
    TEST_CHECK(expr != nullptr, "Expression parsed");

    auto *bin = dynamic_cast<BinaryExprAST *>(expr.get());
    TEST_CHECK(bin != nullptr, "Root is BinaryExprAST");
    TEST_CHECK(bin->getOp() == '+', "Root op is '+'");

    auto *lhsNum = dynamic_cast<NumberExprAST *>(bin->getLHS());
    TEST_CHECK(lhsNum != nullptr && lhsNum->getValue() == 2.0, "LHS is 2.0");

    auto *rhsBin = dynamic_cast<BinaryExprAST *>(bin->getRHS());
    TEST_CHECK(rhsBin != nullptr && rhsBin->getOp() == '*', "RHS is BinaryExprAST with '*'");
}

void testParseParenPrecedence() {
    // (2 + 3) * 4 should parse as (2 + 3) * 4
    std::string src = "(2 + 3) * 4";
    Lexer lexer(src);
    Parser parser(lexer);

    auto expr = parser.parseExpression();
    TEST_CHECK(expr != nullptr, "Paren expression parsed");

    auto *bin = dynamic_cast<BinaryExprAST *>(expr.get());
    TEST_CHECK(bin != nullptr && bin->getOp() == '*', "Root op is '*'");

    auto *lhsBin = dynamic_cast<BinaryExprAST *>(bin->getLHS());
    TEST_CHECK(lhsBin != nullptr && lhsBin->getOp() == '+', "LHS is '+'");
}

void testParseUnary() {
    std::string src = "-x + 5";
    Lexer lexer(src);
    Parser parser(lexer);

    auto expr = parser.parseExpression();
    TEST_CHECK(expr != nullptr, "Unary expression parsed");

    auto *bin = dynamic_cast<BinaryExprAST *>(expr.get());
    TEST_CHECK(bin != nullptr && bin->getOp() == '+', "Root op is '+'");

    auto *unary = dynamic_cast<UnaryExprAST *>(bin->getLHS());
    TEST_CHECK(unary != nullptr && unary->getOpcode() == '-', "LHS is unary '-'");
}

void testParseControlFlow() {
    // If expression
    std::string ifSrc = "if x < 10 then 1 else 0";
    Lexer ifLexer(ifSrc);
    Parser ifParser(ifLexer);
    auto ifExpr = ifParser.parseExpression();
    TEST_CHECK(dynamic_cast<IfExprAST *>(ifExpr.get()) != nullptr, "Parsed IfExprAST");

    // For expression
    std::string forSrc = "for i = 1, 10, 2 in i * i";
    Lexer forLexer(forSrc);
    Parser forParser(forLexer);
    auto forExpr = forParser.parseExpression();
    TEST_CHECK(dynamic_cast<ForExprAST *>(forExpr.get()) != nullptr, "Parsed ForExprAST");
}

void testParseFunctions() {
    std::string defSrc = "def add(x y) x + y";
    Lexer defLexer(defSrc);
    Parser defParser(defLexer);
    auto func = defParser.parseDefinition();
    TEST_CHECK(func != nullptr, "Parsed function definition");
    TEST_CHECK(func->getName() == "add", "Function name matches 'add'");
    TEST_CHECK(func->getProto()->getArgCount() == 2, "Function has 2 arguments");

    std::string extSrc = "extern sin(x)";
    Lexer extLexer(extSrc);
    Parser extParser(extLexer);
    auto extProto = extParser.parseExtern();
    TEST_CHECK(extProto != nullptr, "Parsed extern prototype");
    TEST_CHECK(extProto->getName() == "sin", "Extern name is 'sin'");
}

void testParseDiagnostics() {
    DiagnosticEngine diag;
    std::string badSrc = "def (x) x + 1"; // Missing function name
    diag.setSourceText(badSrc);
    Lexer lexer(badSrc, &diag);
    Parser parser(lexer, &diag);

    auto badDef = parser.parseDefinition();
    TEST_CHECK(badDef == nullptr, "Syntax error correctly yields null");
    TEST_CHECK(diag.hasErrors(), "DiagnosticEngine caught error");
    TEST_CHECK(diag.getErrorCount() >= 1, "Diagnostic error count is at least 1");
}

int runParserTests() {
    testParseBinaryPrecedence();
    testParseParenPrecedence();
    testParseUnary();
    testParseControlFlow();
    testParseFunctions();
    testParseDiagnostics();

    if (gFailedParserTests == 0) {
        std::cout << " [PASS] Parser unit tests passed successfully.\n";
        return 0;
    } else {
        std::cerr << " [FAIL] Parser unit tests failed with " << gFailedParserTests << " error(s).\n";
        return 1;
    }
}
