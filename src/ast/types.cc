#include "carl/ast/types.h"

#include <vector>

using namespace carl;
using namespace carl::types;

BaseType Unknown::get_base_type() { return BaseType::UNKNOWN; }
std::string Unknown::str() const { return std::string("?"); }

BaseType Void::get_base_type() { return BaseType::VOID; }
std::string Void::str() const { return std::string("void"); }

bool Number::is_number() { return true; }

BaseType Int::get_base_type() { return BaseType::INT; }
bool Int::can_cast_to(Type* other) {
    if (other->get_base_type() == BaseType::FLOAT) return true;
    return get_base_type() == other->get_base_type();
}
std::string Int::str() const { return std::string("int"); }
llvm::Type* Int::get_llvm_rt_type(llvm::LLVMContext& ctx) {
    return llvm::Type::getInt64Ty(ctx);
}

BaseType Float::get_base_type() { return BaseType::FLOAT; }
std::string Float::str() const { return std::string("float"); }
llvm::Type* Float::get_llvm_rt_type(llvm::LLVMContext& ctx) {
    return llvm::Type::getDoubleTy(ctx);
}

BaseType Bool::get_base_type() { return BaseType::BOOL; }
std::string Bool::str() const { return std::string("bool"); }
llvm::Type* Bool::get_llvm_rt_type(llvm::LLVMContext& ctx) {
    return llvm::Type::getInt1Ty(ctx);
}

BaseType String::get_base_type() { return BaseType::STRING; }
std::string String::str() const { return std::string("string"); }
llvm::Type* String::get_llvm_rt_type(llvm::LLVMContext& ctx) {
    llvm::Type* t = llvm::StructType::getTypeByName(ctx, llvm_type_name);
    if (t == nullptr) {
        t = llvm::StructType::create(llvm_type_name,
                                     llvm::Type::getInt64Ty(ctx),
                                     llvm::Type::getInt8PtrTy(ctx));
    }
    return t;
}
bool String::is_rt_heap_obj() { return true; }

Fn::Fn() : parameters({}), ret(std::make_shared<Void>()) {}
Fn::Fn(std::vector<std::shared_ptr<Type>> parameters)
    : parameters(parameters), ret(std::make_shared<Void>()) {}
Fn::Fn(std::vector<std::shared_ptr<Type>> parameters, std::shared_ptr<Type> ret)
    : parameters(parameters), ret(ret){};
BaseType Fn::get_base_type() { return BaseType::FN; }
llvm::Type* Fn::get_llvm_rt_type(llvm::LLVMContext& ctx) {
    llvm::Type* t = llvm::StructType::getTypeByName(ctx, llvm_type_name);
    if (t == nullptr) {
        t = llvm::StructType::create(llvm_type_name, llvm::PointerType::get(ctx, 0));
    }
    return t;
}
llvm::FunctionType* Fn::get_llvm_fn_type(llvm::LLVMContext& ctx) {
    auto ret_type = ret->get_llvm_rt_type(ctx);
    std::vector<llvm::Type*> fps;
    for (auto& param : parameters) {
        auto *x = param->get_llvm_rt_type(ctx);
        // we pass heap objects by reference
        if (param->is_rt_heap_obj()) x = x->getPointerTo(0);
        fps.push_back(x);
    }
    return llvm::FunctionType::get(ret_type, fps, false);
}
bool Fn::is_rt_heap_obj() { return true; }

bool Fn::equals(Type* other) {
    if (other->get_base_type() != BaseType::FN) { return false; }

    auto *other_fn_t = static_cast<Fn*>(other);
    if (parameters.size() != other_fn_t->parameters.size()) { return false; }

    for (int i = 0; i < parameters.size(); ++i) {
        auto my_param_t = parameters.at(i);
        auto other_param_t = other_fn_t->parameters.at(i);
        if (!my_param_t->can_cast_to(other_param_t.get())) { return false; }
    }

    if (!ret->can_cast_to(other_fn_t->ret.get())) { return false; }

    return true;
}

bool Fn::can_cast_to(Type* other) {
    return equals(other);
}

const std::vector<std::shared_ptr<Type>> Fn::get_parameters() {
    return parameters;
}
const std::shared_ptr<Type> Fn::get_ret() { return ret; }

bool Fn::can_apply_to(std::vector<std::shared_ptr<Type>> arguments) {
    // allow exact types only for now:
    if (parameters.size() != arguments.size()) return false;
    for (int i = 0; i < parameters.size(); i++) {
        auto& param = parameters.at(i);
        auto& arg = arguments.at(i);
        if (!(param->equals(arg.get()) || arg->can_cast_to(param.get()))) {
            return false;
        }
    }
    return true;
}

std::string Fn::str() const {
    auto result = std::string("(");
    bool first = true;
    for (auto& param : parameters) {
        if (!first) result += ", ";
        result += param->str();
        first = false;
    }
    result += ") -> " + ret->str();
    return result;
}