#include "carl/ast/types.h"

using namespace carl;
using namespace carl::types;

BaseType Unknown::get_base_type() { return BaseType::UNKNOWN; }
std::string Unknown::str() { return std::string("?"); }

BaseType Void::get_base_type() { return BaseType::VOID; }
std::string Void::str() { return std::string("void"); }

bool Number::is_number() { return true; }

BaseType Int::get_base_type() { return BaseType::INT; }
bool Int::can_cast_to(Type* other) {
    if (other->get_base_type() == BaseType::FLOAT) return true;
    return get_base_type() == other->get_base_type();
}
std::string Int::str() { return std::string("int"); }

BaseType Float::get_base_type() { return BaseType::FLOAT; }
std::string Float::str() { return std::string("float"); }

BaseType Bool::get_base_type() { return BaseType::BOOL; }
std::string Bool::str() { return std::string("bool"); }

BaseType String::get_base_type() { return BaseType::STRING; }
std::string String::str() { return std::string("string"); }

Fn::Fn() : parameters({}), ret(std::make_shared<Void>()) {}
Fn::Fn(std::vector<std::shared_ptr<Type>> parameters)
    : parameters(parameters), ret(std::make_shared<Void>()) {}
Fn::Fn(std::vector<std::shared_ptr<Type>> parameters, std::shared_ptr<Type> ret)
    : parameters(parameters), ret(ret){};
BaseType Fn::get_base_type() { return BaseType::FN; }

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

std::string Fn::str() {
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