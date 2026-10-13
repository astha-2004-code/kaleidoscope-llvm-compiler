#include "kaleidoscope/Driver.hpp"
#include <iostream>
#include <string>
#include <vector>

void printHelp(const char *progName) {
    std::cout << "Usage: " << progName << " [options] [source-file]\n\n"
              << "Options:\n"
              << "  -h, --help        Display this help message\n"
              << "  -v, --version     Display compiler version\n"
              << "  --dump-ast        Print formatted Abstract Syntax Tree representation\n"
              << "  --emit-llvm       Emit LLVM Intermediate Representation (IR)\n"
              << "  --no-ast-opt      Disable AST-level constant folding and algebraic simplification\n"
              << "  -O0               Disable optimizations\n"
              << "  -O1               Basic peephole and CFG optimizations\n"
              << "  -O2               Standard optimization pipeline (InstCombine, Reassociate, GVN, CFGSimplification)\n"
              << "  -O3               Aggressive optimization passes\n\n"
              << "If no source file is provided, Kaleidoscope launches in interactive REPL mode.\n";
}

int main(int argc, char **argv) {
    kaleidoscope::CompilerOptions opts;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "Kaleidoscope Modern C++ Compiler v1.0.0\n";
#if KALEIDOSCOPE_HAS_LLVM
            std::cout << "Target Backend: Native LLVM IR & ORC JIT Engine\n";
#else
            std::cout << "Target Backend: Portable LLVM IR Emitting Engine\n";
#endif
            return 0;
        } else if (arg == "--dump-ast") {
            opts.dumpAST = true;
        } else if (arg == "--emit-llvm") {
            opts.emitLLVM = true;
        } else if (arg == "--no-ast-opt") {
            opts.optimizeAST = false;
        } else if (arg == "-O0") {
            opts.optLevel = kaleidoscope::OptLevel::O0;
        } else if (arg == "-O1") {
            opts.optLevel = kaleidoscope::OptLevel::O1;
        } else if (arg == "-O2") {
            opts.optLevel = kaleidoscope::OptLevel::O2;
        } else if (arg == "-O3") {
            opts.optLevel = kaleidoscope::OptLevel::O3;
        } else if (arg[0] == '-') {
            std::cerr << "error: Unknown command line option '" << arg << "'\n";
            printHelp(argv[0]);
            return 1;
        } else {
            opts.inputFile = arg;
        }
    }

    kaleidoscope::Driver driver;
    return driver.run(opts);
}
