#include "carl/llvm/jit.h"

#include <cstdint>

using namespace carl;

extern "C" {
    void* my_malloc(size_t s) {
        auto m = malloc(s);
        return m;
    }

    void debug(int(*memory)(const char*)) {
        return;
    }

    int my_puts(__carl_string* s) {
        int m = puts(s->str);
        return m;
    }
}

LLJITWrapper::LLJITWrapper() {
    llvm::orc::LLJITBuilder builder;
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    lljit = exitErr(llvm::orc::LLJITBuilder().create());

    // register mandatory external functions:
    register_host_function("__malloc", (void*)my_malloc);
    register_host_function("__debug", (void*)debug);
    register_host_function("__free", (void*)free);
    register_host_function("__puts_impl", (void*)(my_puts));
}

void LLJITWrapper::register_host_function(const char* name, void *addr) {
    // more or less this:
    // https://llvm.org/docs/ORCv2.html#how-to-add-process-and-library-symbols-to-jitdylibs

    llvm::orc::MangleAndInterner mangler(lljit->getExecutionSession(), lljit->getDataLayout());

    llvm::orc::SymbolStringPtr sym_name = lljit->getExecutionSession().intern(name);
    llvm::JITEvaluatedSymbol sym(llvm::pointerToJITTargetAddress(addr),
                                 llvm::JITSymbolFlags::Callable | llvm::JITSymbolFlags::Exported | llvm::JITSymbolFlags::Absolute);
    llvm::orc::SymbolMap map {{ sym_name, sym }};
    auto error = lljit->getMainJITDylib().define(llvm::orc::absoluteSymbols(map));
    if (error) {
        llvm::errs() << "could not register host function";
    }
}

std::optional<llvm::orc::ResourceTrackerSP> LLJITWrapper::load_module(llvm::orc::ThreadSafeModule &module) {
    auto tracker = lljit->getMainJITDylib().createResourceTracker();
    auto err = lljit->addIRModule(tracker, std::move(module));
    if (!err) {
        return tracker;
    } else {
        return std::nullopt;
    }
}

std::optional<llvm::orc::ExecutorAddr> LLJITWrapper::lookup_ea(const char* name) {
    auto ea = lljit->lookup(name);
    if (!ea) {
        return {};
    } else {
        return ea.get();
    }
}

std::optional<void*> LLJITWrapper::lookup(const char* name) {
    auto ea = lljit->lookup(name);
    if (!ea) {
        return nullptr;
    } else {
        return ea.get().toPtr<void*>();
    }
}