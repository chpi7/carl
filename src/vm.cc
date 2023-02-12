#include "carl/vm/vm.h"

#include "carl/vm/instruction.h"
#include "carl/vm/memory.h"

using namespace carl;

VM::VM(uint64_t stack_size) : stack_size(stack_size) {
    stack = new carl_stackelem_t[stack_size];
    sp = stack;
}

VM::~VM() {
    if (stack != nullptr) {
        delete[] stack;

        stack = nullptr;
        sp = nullptr;
    }
}

void VM::load_binary(carl_pointer_t start, uint64_t size) {
    // TODO
    ip = start;
}