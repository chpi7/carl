#ifndef CARL_VM_ENV_H
#define CARL_VM_ENV_H

#include <memory>
#include <unordered_map>

#include "carl/vm/memory.h"

namespace carl {
class Env {
   private:
    std::unordered_map<const char*, Value*> values;
    std::unique_ptr<Env> parent;

   public:
    Env() { };
    Env(std::unique_ptr<Env> parent) : parent(std::move(parent)) {}
    // lookup based on address of the name
    Value* lookup(const char* name);
    void insert(const char* name, Value* value);
    void remove(const char* name);
    std::unique_ptr<Env> take_parent();
};
}  // namespace carl

#endif