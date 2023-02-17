#include "carl/vm/vm.h"

#include <iostream>

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
        if (*p == CARL_NIL) {
            fprintf(stdout, "[ nil ]");
        } else {
            fprintf(stdout, "[ %ld ]", *p);
        }
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
        case OP_TRUE:
            push(CARL_TRUE);
            break;
        case OP_FALSE:
            push(CARL_FALSE);
            break;
        case OP_NIL:
            push(CARL_NIL);
            break;
        case OP_MKBASIC: {
            // TODO make basic value of given type
            // maybe just put the type enum value on the stack to get it here transparently
            // pop -> type
            // pop -> value
            // make_basic(type, value) --> address
            // string will be its own type --> mkstring or use vector type (mb vec not good if elements are other heap elems instead of chars...)
            break;
        }
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
        case OP_REM: BINOP(%); break;
        case OP_NEG: push(-pop()); break;
        case OP_AND: {
            auto rhs = pop();
            auto lhs = pop();
            push(logic_binop(OP_AND, lhs, rhs));
            break;
        }
        case OP_OR: {
            auto rhs = pop();
            auto lhs = pop();
            push(logic_binop(OP_OR, lhs, rhs));
            break;
        }
        case OP_NOT: {
            auto lhs = pop();
            push(logic_binop(OP_NOT, lhs, 0));
            break;
        }
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

carl_int_t VM::logic_binop(OpCode op, carl_int_t lhs, carl_int_t rhs) {
    bool lb = static_cast<bool>(lhs);
    bool rb = static_cast<bool>(rhs);
    switch (op) {
        case OP_AND:
            if (lhs == CARL_NIL) return lhs;
            return !lb ? lhs : rhs; // if lhs is true, return rhs for (false, true, nil).
        case OP_OR:
            if (lb) return lhs;
            return rhs; // always return rhs for (false, true, nil).
        case OP_NOT:
            return lhs == CARL_NIL ? lhs : static_cast<carl_int_t>(!lb);
        default:
            std::cerr << "unsupported logical_binop" << std::endl;
    }
}
