#include "kaleidoscope/Optimizer.hpp"

#if KALEIDOSCOPE_HAS_LLVM
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Utils.h>
#endif

namespace kaleidoscope {

PassPipeline::PassPipeline(OptLevel level) : level_(level) {}

PassPipeline::~PassPipeline() = default;

#if KALEIDOSCOPE_HAS_LLVM

void PassPipeline::initialize(llvm::Module *module) {
    if (!module) return;

    fpm_ = std::make_unique<llvm::legacy::FunctionPassManager>(module);

    if (level_ != OptLevel::O0) {
        // Do simple "peephole" optimizations and bit-twiddling
        fpm_->add(llvm::createInstructionCombiningPass());

        // Reassociate expressions to expose parallelism and simplify terms
        fpm_->add(llvm::createReassociatePass());

        // Eliminate Common SubExpressions (CSE / GVN)
        fpm_->add(llvm::createGVNPass());

        // Simplify control flow graph (deleting unreachable blocks, merging blocks)
        fpm_->add(llvm::createCFGSimplificationPass());
    }

    fpm_->doInitialization();
}

bool PassPipeline::run(llvm::Function &function) {
    if (fpm_ && level_ != OptLevel::O0) {
        return fpm_->run(function);
    }
    return false;
}

#else

bool PassPipeline::run() {
    return true;
}

#endif

} // namespace kaleidoscope
