#ifndef KALEIDOSCOPE_CODEGEN_HPP
#define KALEIDOSCOPE_CODEGEN_HPP

#include "Common.hpp"
#include "AST.hpp"
#include "Diagnostic.hpp"
#include <string>
#include <map>
#include <memory>
#include <iostream>

#if KALEIDOSCOPE_HAS_LLVM
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#endif

namespace kaleidoscope {

class IRGenerator {
public:
    explicit IRGenerator(const std::string &moduleName = "kaleidoscope_module", DiagnosticEngine *diag = nullptr);
    ~IRGenerator();

    void dumpIR(std::ostream &os = std::cout);
    std::string getIRString();

#if KALEIDOSCOPE_HAS_LLVM
    llvm::LLVMContext &getContext() { return context_; }
    llvm::Module *getModule() { return module_.get(); }
    llvm::IRBuilder<> &getBuilder() { return builder_; }

    llvm::Value *codegen(ExprAST &expr);
    llvm::Function *codegen(PrototypeAST &proto);
    llvm::Function *codegen(FunctionAST &func);

    llvm::Function *getFunction(const std::string &name);
#else
    bool codegen(ExprAST &expr, std::ostream &os = std::cout);
    bool codegen(PrototypeAST &proto, std::ostream &os = std::cout);
    bool codegen(FunctionAST &func, std::ostream &os = std::cout);
#endif

    void resetModule(const std::string &moduleName);

private:
    std::string moduleName_;
    DiagnosticEngine *diag_{nullptr};

#if KALEIDOSCOPE_HAS_LLVM
    llvm::LLVMContext context_;
    llvm::IRBuilder<> builder_;
    std::unique_ptr<llvm::Module> module_;
    std::map<std::string, llvm::Value *> namedValues_;
    std::map<std::string, std::unique_ptr<PrototypeAST>> functionProtos_;
#else
    std::map<std::string, std::unique_ptr<PrototypeAST>> functionProtos_;
    std::string textIRBuffer_;
#endif
};

} // namespace kaleidoscope

#endif // KALEIDOSCOPE_CODEGEN_HPP
