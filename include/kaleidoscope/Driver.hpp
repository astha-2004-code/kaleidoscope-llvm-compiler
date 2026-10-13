#ifndef KALEIDOSCOPE_DRIVER_HPP
#define KALEIDOSCOPE_DRIVER_HPP

#include "Common.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"
#include "ASTVisitor.hpp"
#include "ASTOptimizer.hpp"
#include "CodeGen.hpp"
#include "Optimizer.hpp"
#include "Diagnostic.hpp"
#include <string>
#include <istream>

namespace kaleidoscope {

struct CompilerOptions {
    std::string inputFile;
    bool dumpAST{false};
    bool emitLLVM{false};
    bool runREPL{false};
    bool optimizeAST{true};
    OptLevel optLevel{OptLevel::O2};
};

class Driver {
public:
    Driver();
    ~Driver();

    int run(const CompilerOptions &opts);
    int compileStream(std::istream &in, const CompilerOptions &opts);
    int runREPL(const CompilerOptions &opts);

private:
    DiagnosticEngine diag_;
    IRGenerator irGen_;
    PassPipeline passPipeline_;
    ASTConstantFolder astOpt_;
    ASTDumper dumper_;
};

} // namespace kaleidoscope

#endif // KALEIDOSCOPE_DRIVER_HPP
