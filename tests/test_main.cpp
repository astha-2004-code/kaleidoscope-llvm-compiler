#include <iostream>

// Forward declarations of test suite runners
int runLexerTests();
int runParserTests();
int runASTOptimizerTests();
int runCodeGenTests();

int main() {
    std::cout << "==========================================\n";
    std::cout << " Running Kaleidoscope Test Suite\n";
    std::cout << "==========================================\n";

    int failures = 0;

    std::cout << "\n[1/4] Running Lexer Unit Tests...\n";
    failures += runLexerTests();

    std::cout << "\n[2/4] Running Parser Unit Tests...\n";
    failures += runParserTests();

    std::cout << "\n[3/4] Running AST Optimizer Unit Tests...\n";
    failures += runASTOptimizerTests();

    std::cout << "\n[4/4] Running CodeGen Unit Tests...\n";
    failures += runCodeGenTests();

    std::cout << "\n==========================================\n";
    if (failures == 0) {
        std::cout << " ALL TESTS PASSED SUCCESSFULLY! (0 failures)\n";
        std::cout << "==========================================\n";
        return 0;
    } else {
        std::cerr << " TEST SUITE FAILED WITH " << failures << " ERRORS!\n";
        std::cout << "==========================================\n";
        return 1;
    }
}
