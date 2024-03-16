#include "carl/jit2/carljit.h"

#include "carl/jit2/runtime.h"

#include <cstdint>

using namespace carl;

static CarlJIT* CURRENT_JIT_PTR = nullptr;

extern "C" {
    void carl_debug(uint64_t data) {
        if (CURRENT_JIT_PTR) CURRENT_JIT_PTR->debug_values.push_back(data);
        return;
    }
}

CarlJIT::CarlJIT() {
    llvm::orc::LLJITBuilder builder;
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    lljit = exitErr(llvm::orc::LLJITBuilder().create());

    // register mandatory external functions:
    // register_host_function("__malloc", (void*)my_malloc);
    register_host_function("__debug", (void*)carl_debug);
    register_host_function("crt_malloc", (void*)crt_malloc);
    register_host_function("crt_string__concat", (void*)crt_string__concat);

    // not sure if this is such a great idea but somehow the functions above
    // need access
    CURRENT_JIT_PTR = this;
}

void CarlJIT::register_host_function(const char* name, void *addr) {
    // more or less this:
    // https://llvm.org/docs/ORCv2.html#how-to-add-process-and-library-symbols-to-jitdylibs

    llvm::orc::MangleAndInterner mangler(lljit->getExecutionSession(), lljit->getDataLayout());

    llvm::orc::SymbolStringPtr sym_name = lljit->getExecutionSession().intern(name);
    llvm::JITEvaluatedSymbol sym(llvm::pointerToJITTargetAddress(addr),
                                 llvm::JITSymbolFlags::Callable | llvm::JITSymbolFlags::Exported | llvm::JITSymbolFlags::Absolute);
    auto error = lljit->getMainJITDylib().define(llvm::orc::absoluteSymbols({{ sym_name, sym }}));
    if (error) {
        llvm::errs() << "could not register host function";
    }
}

void CarlJIT::set_outs(std::ostream* os) {
    this->outs = os;
}

void CarlJIT::write_outs(const char* s) {
    if (outs) {
        (*outs) << s;
    }
}

std::optional<llvm::orc::ResourceTrackerSP> CarlJIT::load_module(Codegen2Module &module) {
    auto tracker = lljit->getMainJITDylib().createResourceTracker();
    auto err = lljit->addIRModule(tracker, module.take_llvm_module());
    if (!err) {
        return tracker;
    } else {
        return std::nullopt;
    }
}

std::optional<llvm::orc::ExecutorAddr> CarlJIT::lookup_ea(const char* name) {
    auto ea = lljit->lookup(name);
    if (!ea) {
        return {};
    } else {
        return ea.get();
    }
}

std::optional<void*> CarlJIT::lookup(const char* name) {
    auto ea = lljit->lookup(name);
    if (!ea) {
        return nullptr;
    } else {
        return ea.get().toPtr<void*>();
    }
}
