#include "carl/llvm/jit.h"

using namespace carl;

std::optional<llvm::orc::ResourceTrackerSP> LLJITWrapper::load_module(llvm::orc::ThreadSafeModule &module) {
    auto tracker = lljit->getMainJITDylib().createResourceTracker();
    auto err = lljit->addIRModule(tracker, std::move(module));
    if (!err) {
        return tracker;
    } else {
        return std::nullopt;
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