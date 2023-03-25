#include "carl/llvm/jit.h"

#include <cstdint>

using namespace carl;

static LLJITWrapper* CURRENT_JIT_PTR = nullptr;

extern "C" {
    void* my_malloc(size_t s) {
        auto m = malloc(s);
        return m;
    }

    void debug(uint64_t ptr) {
        if (CURRENT_JIT_PTR) CURRENT_JIT_PTR->debug_values.push_back(ptr);
        return;
    }

    int my_puts(__carl_string* s) {
        // int m = fputs(s->str, stdout);
        if (CURRENT_JIT_PTR) CURRENT_JIT_PTR->write_outs(s->str);
        // "A nonnegative value indicates that no error has occured."
        return 0;
    }
}

LLJITWrapper::LLJITWrapper() {
    llvm::orc::LLJITBuilder builder;
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    lljit = exitErr(llvm::orc::LLJITBuilder().create());

    // register mandatory external functions:
    register_host_function("__malloc", (void*)my_malloc);
    register_host_function("__free", (void*)free);
    register_host_function("__debug_impl", (void*)debug);
    register_host_function("__puts_impl", (void*)(my_puts));

    // not sure if this is such a great idea but somehow the functions above
    // need access
    CURRENT_JIT_PTR = this;
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

void LLJITWrapper::set_outs(std::ostream* os) {
    this->outs = os;
}

void LLJITWrapper::write_outs(const char* s) {
    if (outs) {
        (*outs) << s;
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