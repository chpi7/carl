#include "carl/vm/env.h"

#include <iostream>

using namespace carl;

Value* Env::lookup(const char* key) {
    if (values.contains(key)) {
        return values.at(key);
    } else if (parent) {
        return parent->lookup(key);
    } else {
        std::cerr << std::string(key) << " is not defined (Env::lookup).\n";
        return nullptr;
    }
}

void Env::insert(const char* key, Value* value) {
    values[key] = value;
}

void Env::remove(const char* key) {
    if (values.contains(key)) {
        values.erase(key);
    } else {
        std::cerr << std::string(key) << " is not defined (Env::remove).\n";
    }
}

void Env::update(const char* key, Value* value) {
    auto current_value = lookup(key);
    if (current_value == nullptr) {
        std::cerr << std::string(key) << " is not defined (Env::update).\n";
        return;
    }
    values[key] = value;
}

std::unique_ptr<Env> Env::take_parent() {
    if (!parent) return std::make_unique<Env>();
    return std::move(parent);
}
