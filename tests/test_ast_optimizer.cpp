#include "kaleidoscope/Lexer.hpp"
#include "kaleidoscope/Parser.hpp"
#include "kaleidoscope/ASTOptimizer.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace kaleidoscope;

static int gFailedOptTests = 0;

#define TEST_OPT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "OPT TEST FAILED: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            ++gFailedOptTests; \
        } \
    } while(0)

void testConstantFoldingArithmetic() {
    // 2 + 3 * 4 -> 14
    std::string src = "2 + 3 * 4";
    Lexer lexer(src);
    Parser parser(lexer);
    auto expr = parser.parseExpression();

    ASTConstantFolder opt;
    auto folded = opt.optimize(std::move(expr));

    auto *num = dynamic_cast<NumberExprAST *>(folded.get());
    TEST_OPT(num != nullptr, "Folded to a NumberExprAST");
    TEST_OPT(num && std::fabs(num->getValue() - 14.0) < 1e-6, "2 + 3 * 4 folded to 14.0");
}

void testConstantFoldingComparison() {
    // 5 < 10 -> 1.0
    std::string src1 = "5 < 10";
    Lexer lexer1(src1);
    Parser parser1(lexer1);
    ASTConstantFolder opt;
    auto folded1 = opt.optimize(parser1.parseExpression());
    auto *num1 = dynamic_cast<NumberExprAST *>(folded1.get());
    TEST_OPT(num1 && num1->getValue() == 1.0, "5 < 10 folded to 1.0");

    // 10 < 5 -> 0.0
    std::string src2 = "10 < 5";
    Lexer lexer2(src2);
    Parser parser2(lexer2);
    auto folded2 = opt.optimize(parser2.parseExpression());
    auto *num2 = dynamic_cast<NumberExprAST *>(folded2.get());
    TEST_OPT(num2 && num2->getValue() == 0.0, "10 < 5 folded to 0.0");
}

void testAlgebraicIdentities() {
    ASTConstantFolder opt;

    // x + 0 -> x
    {
        std::string src = "x + 0";
        Lexer lex(src); Parser p(lex);
        auto folded = opt.optimize(p.parseExpression());
        auto *var = dynamic_cast<VariableExprAST *>(folded.get());
        TEST_OPT(var && var->getName() == "x", "x + 0 folded to x");
    }

    // 0 + x -> x
    {
        std::string src = "0 + x";
        Lexer lex(src); Parser p(lex);
        auto folded = opt.optimize(p.parseExpression());
        auto *var = dynamic_cast<VariableExprAST *>(folded.get());
        TEST_OPT(var && var->getName() == "x", "0 + x folded to x");
    }

    // x * 1 -> x
    {
        std::string src = "x * 1";
        Lexer lex(src); Parser p(lex);
        auto folded = opt.optimize(p.parseExpression());
        auto *var = dynamic_cast<VariableExprAST *>(folded.get());
        TEST_OPT(var && var->getName() == "x", "x * 1 folded to x");
    }

    // x * 0 -> 0
    {
        std::string src = "x * 0";
        Lexer lex(src); Parser p(lex);
        auto folded = opt.optimize(p.parseExpression());
        auto *num = dynamic_cast<NumberExprAST *>(folded.get());
        TEST_OPT(num && num->getValue() == 0.0, "x * 0 folded to 0.0");
    }

    // x / 1 -> x
    {
        std::string src = "x / 1";
        Lexer lex(src); Parser p(lex);
        auto folded = opt.optimize(p.parseExpression());
        auto *var = dynamic_cast<VariableExprAST *>(folded.get());
        TEST_OPT(var && var->getName() == "x", "x / 1 folded to x");
    }
}

void testDeadBranchElimination() {
    ASTConstantFolder opt;

    // if 1 then x + 5 else y + 10 -> x + 5
    {
        std::string src = "if 1 then x + 5 else y + 10";
        Lexer lex(src); Parser p(lex);
        auto folded = opt.optimize(p.parseExpression());
        auto *bin = dynamic_cast<BinaryExprAST *>(folded.get());
        TEST_OPT(bin != nullptr && bin->getOp() == '+', "Dead else branch eliminated");
        auto *var = dynamic_cast<VariableExprAST *>(bin->getLHS());
        TEST_OPT(var && var->getName() == "x", "Resulting expression evaluates 'then' branch");
    }

    // if 0 then x + 5 else y + 10 -> y + 10
    {
        std::string src = "if 0 then x + 5 else y + 10";
        Lexer lex(src); Parser p(lex);
        auto folded = opt.optimize(p.parseExpression());
        auto *bin = dynamic_cast<BinaryExprAST *>(folded.get());
        TEST_OPT(bin != nullptr && bin->getOp() == '+', "Dead then branch eliminated");
        auto *var = dynamic_cast<VariableExprAST *>(bin->getLHS());
        TEST_OPT(var && var->getName() == "y", "Resulting expression evaluates 'else' branch");
    }
}

int runASTOptimizerTests() {
    testConstantFoldingArithmetic();
    testConstantFoldingComparison();
    testAlgebraicIdentities();
    testDeadBranchElimination();

    if (gFailedOptTests == 0) {
        std::cout << " [PASS] AST Optimizer unit tests passed successfully.\n";
        return 0;
    } else {
        std::cerr << " [FAIL] AST Optimizer unit tests failed with " << gFailedOptTests << " error(s).\n";
        return 1;
    }
}
