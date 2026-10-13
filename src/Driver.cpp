#include "kaleidoscope/Driver.hpp"
#include <fstream>
#include <sstream>

namespace kaleidoscope {

Driver::Driver()
    : irGen_("kaleidoscope_module", &diag_),
      passPipeline_(OptLevel::O2) {}

Driver::~Driver() = default;

int Driver::run(const CompilerOptions &opts) {
    passPipeline_.setLevel(opts.optLevel);

    if (opts.runREPL || opts.inputFile.empty()) {
        return runREPL(opts);
    }

    std::ifstream file(opts.inputFile);
    if (!file.is_open()) {
        std::cerr << "error: Could not open input file: " << opts.inputFile << "\n";
        return 1;
    }

    // Read full source for diagnostic snippet reporting
    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    diag_.setSourceText(source);

    std::istringstream stream(source);
    return compileStream(stream, opts);
}

int Driver::compileStream(std::istream &in, const CompilerOptions &opts) {
#if KALEIDOSCOPE_HAS_LLVM
    passPipeline_.initialize(irGen_.getModule());
#endif

    Lexer lexer(in, &diag_);
    Parser parser(lexer, &diag_);

    while (parser.currentToken().isNot(TokenKind::EndOfFile)) {
        if (parser.currentToken().is(TokenKind::Semicolon)) {
            parser.getNextToken(); // Ignore top-level semicolon
            continue;
        }

        if (parser.currentToken().is(TokenKind::Def)) {
            auto func = parser.parseDefinition();
            if (!func) {
                parser.synchronize();
                continue;
            }

            if (opts.optimizeAST) {
                astOpt_.optimize(*func);
            }

            if (opts.dumpAST) {
                dumper_.dump(*func);
            }

#if KALEIDOSCOPE_HAS_LLVM
            if (llvm::Function *lf = irGen_.codegen(*func)) {
                passPipeline_.run(*lf);
            }
#else
            if (opts.emitLLVM) {
                irGen_.codegen(*func, std::cout);
            }
#endif
        } else if (parser.currentToken().is(TokenKind::Extern)) {
            auto proto = parser.parseExtern();
            if (!proto) {
                parser.synchronize();
                continue;
            }

            if (opts.dumpAST) {
                dumper_.dump(*proto);
            }

#if KALEIDOSCOPE_HAS_LLVM
            irGen_.codegen(*proto);
#else
            if (opts.emitLLVM) {
                irGen_.codegen(*proto, std::cout);
            }
#endif
        } else {
            // Top-level anonymous expression
            auto func = parser.parseTopLevelExpr();
            if (!func) {
                parser.synchronize();
                continue;
            }

            if (opts.optimizeAST) {
                astOpt_.optimize(*func);
            }

            if (opts.dumpAST) {
                dumper_.dump(*func);
            }

#if KALEIDOSCOPE_HAS_LLVM
            if (llvm::Function *lf = irGen_.codegen(*func)) {
                passPipeline_.run(*lf);
            }
#else
            if (opts.emitLLVM) {
                irGen_.codegen(*func, std::cout);
            }
#endif
        }
    }

#if KALEIDOSCOPE_HAS_LLVM
    if (opts.emitLLVM) {
        irGen_.dumpIR(std::cout);
    }
#endif

    return diag_.hasErrors() ? 1 : 0;
}

int Driver::runREPL(const CompilerOptions &opts) {
    std::cout << "Kaleidoscope Modern Compiler REPL (type 'def', 'extern', or expression)\n";
    std::cout << "Type Ctrl+D (or Ctrl+Z on Windows) to exit.\n";

#if KALEIDOSCOPE_HAS_LLVM
    passPipeline_.initialize(irGen_.getModule());
#endif

    std::string line;
    std::cout << "ready> ";
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            std::cout << "ready> ";
            continue;
        }

        diag_.setSourceText(line);
        std::istringstream stream(line);
        compileStream(stream, opts);
        std::cout << "ready> ";
    }
    std::cout << "\nExiting REPL.\n";
    return 0;
}

} // namespace kaleidoscope
