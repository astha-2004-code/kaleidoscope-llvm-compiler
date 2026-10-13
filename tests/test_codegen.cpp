#include "kaleidoscope/Lexer.hpp"
#include "kaleidoscope/Parser.hpp"
#include "kaleidoscope/CodeGen.hpp"
#include <cassert>
#include <iostream>
#include <sstream>

using namespace kaleidoscope {

static int gFailedCodeGenTests = 0;

#define TEST_CODEGEN(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "CODEGEN TEST FAILED: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            ++gFailedCodeGenTests; \
        } \
    } while(0)

void testCodeGenFunctionDefinition() {
    std::string src = "def add(x y) x + y";
    Lexer lexer(src);
    Parser parser(lexer);
    auto func = parser.parseDefinition();
    TEST_CODEGEN(func != nullptr, "Parsed add function");

    IRGenerator gen("test_module");
    std::ostringstream ss;
#if KALEIDOSCOPE_HAS_LLVM
    llvm::Function *llvmFunc = gen.codegen(*func);
    TEST_CODEGEN(llvmFunc != nullptr, "Emitted LLVM function");
    gen.dumpIR(ss);
#else
    gen.codegen(*func, ss);
#endif
    std::string ir = ss.str();
    TEST_CODEGEN(ir.find("define double @add") != std::string::npos, "IR contains 'define double @add'");
    TEST_CODEGEN(ir.find("fadd double") != std::string::npos, "IR contains 'fadd double'");
    TEST_CODEGEN(ir.find("ret double") != std::string::npos, "IR contains 'ret double'");
}

void testCodeGenConditionals() {
    std::string src = "def min(x y) if x < y then x else y";
    Lexer lexer(src);
    Parser parser(lexer);
    auto func = parser.parseDefinition();
    TEST_CODEGEN(func != nullptr, "Parsed min function");

    IRGenerator gen("test_cond_module");
    std::ostringstream ss;
#if KALEIDOSCOPE_HAS_LLVM
    llvm::Function *llvmFunc = gen.codegen(*func);
    TEST_CODEGEN(llvmFunc != nullptr, "Emitted LLVM conditional function");
    gen.dumpIR(ss);
#else
    gen.codegen(*func, ss);
#endif
    std::string ir = ss.str();
    TEST_CODEGEN(ir.find("define double @min") != std::string::npos, "IR contains 'define double @min'");
    TEST_CODEGEN(ir.find("phi double") != std::string::npos, "IR contains PHI node for merge");
}

void testCodeGenExtern() {
    std::string src = "extern putchard(x)";
    Lexer lexer(src);
    Parser parser(lexer);
    auto proto = parser.parseExtern();
    TEST_CODEGEN(proto != nullptr, "Parsed extern prototype");

    IRGenerator gen("test_extern_module");
    std::ostringstream ss;
#if KALEIDOSCOPE_HAS_LLVM
    llvm::Function *llvmProto = gen.codegen(*proto);
    TEST_CODEGEN(llvmProto != nullptr, "Emitted LLVM extern declaration");
    gen.dumpIR(ss);
#else
    gen.codegen(*proto, ss);
#endif
    std::string ir = ss.str();
    TEST_CODEGEN(ir.find("declare double @putchard") != std::string::npos, "IR contains 'declare double @putchard'");
}

int runCodeGenTests() {
    testCodeGenFunctionDefinition();
    testCodeGenConditionals();
    testCodeGenExtern();

    if (gFailedCodeGenTests == 0) {
        std::cout << " [PASS] CodeGen unit tests passed successfully.\n";
        return 0;
    } else {
        std::cerr << " [FAIL] CodeGen unit tests failed with " << gFailedCodeGenTests << " error(s).\n";
        return 1;
    }
}
