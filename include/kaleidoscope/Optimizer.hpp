#ifndef KALEIDOSCOPE_OPTIMIZER_HPP
#define KALEIDOSCOPE_OPTIMIZER_HPP

#include "Common.hpp"
#include <string>
#include <memory>

#if KALEIDOSCOPE_HAS_LLVM
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#endif

namespace kaleidoscope {

enum class OptLevel {
    O0,
    O1,
    O2,
    O3
};

class PassPipeline {
public:
    explicit PassPipeline(OptLevel level = OptLevel::O2);
    ~PassPipeline();

    void setLevel(OptLevel level) { level_ = level; }
    OptLevel getLevel() const { return level_; }

#if KALEIDOSCOPE_HAS_LLVM
    void initialize(llvm::Module *module);
    bool run(llvm::Function &function);
#else
    bool run();
#endif

private:
    OptLevel level_;

#if KALEIDOSCOPE_HAS_LLVM
    std::unique_ptr<llvm::legacy::FunctionPassManager> fpm_;
#endif
};

} // namespace kaleidoscope

#endif // KALEIDOSCOPE_OPTIMIZER_HPP
