#ifndef carl_llvm_jit_h
#define carl_llvm_jit_h

#include <memory>

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"

namespace carl {
class JIT {
   private:
    std::unique_ptr<llvm::orc::ExecutionSession> session;

    llvm::DataLayout data_layout;
    llvm::orc::MangleAndInterner mangle;

    llvm::orc::RTDyldObjectLinkingLayer object_layer;
    llvm::orc::IRCompileLayer compile_layer;

    llvm::orc::JITDylib &main_jd;

   public:
    JIT(std::unique_ptr<llvm::orc::ExecutionSession> session,
        llvm::orc::JITTargetMachineBuilder builder, llvm::DataLayout layout)
        : session(std::move(session)),
          data_layout(std::move(layout)),
          mangle(*this->session, this->data_layout),
          object_layer(
              *this->session,
              []() { return std::make_unique<llvm::SectionMemoryManager>(); }),
          compile_layer(*this->session, object_layer,
                        std::make_unique<llvm::orc::ConcurrentIRCompiler>(
                            std::move(builder))),
          main_jd(this->session->createBareJITDylib("<main>")) {
        main_jd.addGenerator(llvm::cantFail(
            llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
                data_layout.getGlobalPrefix())));
    }

    ~JIT() {
        if (auto err = session->endSession()) {
            session->reportError(std::move(err));
        }
    }

    static llvm::Expected<std::unique_ptr<JIT>> Create() {
        auto epc = llvm::orc::SelfExecutorProcessControl::Create();
        if (!epc) {
            return epc.takeError();
        }

        auto session =
            std::make_unique<llvm::orc::ExecutionSession>(std::move(*epc));

        llvm::orc::JITTargetMachineBuilder builder(
            session->getExecutorProcessControl().getTargetTriple());

        auto layout = builder.getDefaultDataLayoutForTarget();
        if (!layout) {
            return layout.takeError();
        }

        return std::make_unique<JIT>(std::move(session), std::move(builder),
                                     std::move(*layout));
    }

    const llvm::DataLayout &getDataLayout() const { return data_layout; }

    llvm::orc::JITDylib &getMainJITDylib() { return main_jd; }

    llvm::Error addModule(llvm::orc::ThreadSafeModule ts_module,
                          llvm::orc::ResourceTrackerSP tracker = nullptr) {
        if (!tracker) {
            tracker = main_jd.getDefaultResourceTracker();
        }
        return compile_layer.add(tracker, std::move(ts_module));
    }

    llvm::Expected<llvm::JITEvaluatedSymbol> lookup(llvm::StringRef name) {
        return session->lookup({&main_jd}, mangle(name.str()));
    }
};
}  // namespace carl

#endif