#ifndef carl_llvm_jit_h
#define carl_llvm_jit_h

#include <memory>
#include <optional>

#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/Support/TargetSelect.h"

namespace carl {
class LLJITWrapper {
    private:
    std::unique_ptr<llvm::orc::LLJIT> lljit;

   public:
    LLJITWrapper() {
        llvm::ExitOnError exitErr;
        llvm::orc::LLJITBuilder builder;
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();

        lljit = exitErr(llvm::orc::LLJITBuilder().create());
    }

    std::optional<llvm::orc::ResourceTrackerSP> load_module(llvm::orc::ThreadSafeModule &module);
    std::optional<void*> lookup(const char* name);
};
}  // namespace carl

#endif