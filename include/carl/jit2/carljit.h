#ifndef carl_jit_h
#define carl_jit_h

#include <memory>
#include <optional>
#include <iostream>
#include <vector>

#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/Support/TargetSelect.h"

#include "carl/jit2/codegen2.h"

namespace carl {

class CarlJIT {
       private:
        std::unique_ptr<llvm::orc::LLJIT> lljit;
        llvm::ExitOnError exitErr;

       public:
        std::ostream* outs = nullptr;
        std::vector<uint64_t> debug_values;

       public:
        CarlJIT();
        void register_host_function(const char* name, void* addr);
        void set_outs(std::ostream* os);
        void write_outs(const char* s);
        std::optional<llvm::orc::ResourceTrackerSP> load_module(Codegen2Module &module);
        std::optional<llvm::orc::ExecutorAddr> lookup_ea(const char* name);
        std::optional<void*> lookup(const char* name);
};
}  // namespace carl

#endif