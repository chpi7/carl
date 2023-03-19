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
    llvm::ExitOnError exitErr;

   public:
    LLJITWrapper();
    void register_host_function(const char* name, void *addr);
    std::optional<llvm::orc::ResourceTrackerSP> load_module(llvm::orc::ThreadSafeModule &module);
    std::optional<llvm::orc::ExecutorAddr> lookup_ea(const char* name);
    std::optional<void*> lookup(const char* name);
};
}  // namespace carl

#endif