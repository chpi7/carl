#include "carl/vm/vm.h"

#include "carl/vm/chunk.h"
#include "carl/vm/memory.h"

using namespace carl;

VM::VM(uint64_t stack_size) : stack_size(stack_size) {
    init_stack();
}

VM::~VM() {
    free_stack();
}

void VM::free_stack() {
    if (stack != nullptr) {
        delete[] stack;

        stack = nullptr;
        sp = nullptr;
    }
}

void VM::init_stack() {
    stack = new carl_stackelem_t[stack_size];
    sp = stack;
}

void VM::print_stack() {
    for (carl_stackelem_t* p = stack + 1; p <= sp; p++) {
        fprintf(stdout, "[ %ld ]", *p);
    }
    fprintf(stdout, "\n");
}

void VM::load_chunk(std::unique_ptr<Chunk> chunk) {
    this->chunk = std::move(chunk);
    ip = this->chunk->get_memory();
}

InterpretResult VM::step() {
#define BINOP(op) do { \
    auto rhs = pop(); \
    auto lhs = pop(); \
    push(lhs op rhs); \
} while (0);
    OpCode op = static_cast<OpCode>(*ip++);
    switch (op) {
        case OP_LOADC: {
            carl_int_t* vp = reinterpret_cast<carl_int_t*>(ip);
            ip += sizeof(carl_int_t);
            push(*vp);
            break;
        }
        case OP_POP: pop(); break;
        case OP_ADD: BINOP(+); break;
        case OP_SUB: BINOP(-); break;
        case OP_MUL: BINOP(*); break;
        case OP_DIV: BINOP(/); break;
        case OP_REM: BINOP(/); break;
        case OP_NEG: push(-pop()); break;
        case OP_HALT: return STEP_HALT;
        default:
            return STEP_ERROR;
    }
    return STEP_OK;
#undef BINOP
}

InterpretResult VM::run() {
    InterpretResult r = STEP_OK; // empty program is okay
    do {
        r = step();
        print_stack();
    } while (r == STEP_OK);
    return r;
}

carl_stackelem_t VM::get_stack_top() {
    return *sp;    
}

carl_stackelem_t VM::pop() {
    carl_stackelem_t v = *sp;
    sp--;
    return v;
}

void VM::push(carl_stackelem_t v) {
    ++sp;
    *sp = v;
}
