#include "carl/vm/vm.h"

#include <iostream>
#include <cstring>

#include "carl/vm/chunk.h"
#include "carl/vm/memory.h"

using namespace carl;

VM::VM(uint64_t stack_size) : stack_size(stack_size) {
    env = std::make_unique<Env>();
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
    if (stack + 1 > sp) {
        fprintf(stdout, "<empty>\n");
        return;
    }
    for (carl_stackelem_t* p = stack + 1; p <= sp; p++) {
        if (*p == CARL_NIL) {
            fprintf(stdout, "[ nil ]");
        } else if (*p < 255) {
            fprintf(stdout, "[ %ld ]", *p);
        } else {
            fprintf(stdout, "[ %lx ]", *p);
        }
    }
    fprintf(stdout, "\n");
}

Value* VM::get_value_by_name(const char* target) {
    for (auto& name : names) {
        if (name->size() == strnlen(target, 20)) {
            if (strncmp(name->c_str(), target, name->size()) == 0) {
                return env->lookup(name->c_str());
            }
        }
    }
    return nullptr;
}

void VM::load_chunk(std::unique_ptr<Chunk> chunk) {
    this->chunk = std::move(chunk);

    for (auto& pair : this->chunk->get_names()) {
        names.insert(pair.second);
    }
    ip = this->chunk->get_memory();
}

InterpretResult VM::step(bool print_trace) {
#define BINOP(op) do { \
    auto rhs = pop(); \
    auto lhs = pop(); \
    if (rhs == CARL_NIL || lhs == CARL_NIL) { \
        push(CARL_NIL); \
    } else { \
        push(lhs op rhs); \
    } \
} while (0);

    if (print_trace) {
        chunk->print_single(std::cout, ip);
        std::cout << std::endl;
    }

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
            push(mkbasic());
            break;
        }
        case OP_GTBASIC: {
            push(gtbasic());
            break;
        }
        case OP_UPDATE_VALUE: {
            update_value();
            break;
        }
        case OP_DEFINE_VAR: {
            // this has to be a valid address to the name string
            const char* name = reinterpret_cast<const char*>(pop());
            Value* value = reinterpret_cast<Value*>(pop());
            env->insert(name, value);
            break;
        }
        case OP_GET_VAR: {
            // this has to be a valid address to the name string
            const char* name = reinterpret_cast<const char*>(pop());
            auto val = reinterpret_cast<carl_stackelem_t>(env->lookup(name));
            // getting a var will always load a heap address --> 0 is invalid
            if (val == 0) {
                push(0);
                return STEP_ERROR;
            }
            push(val);
            break;
        }
        case OP_SET_VAR: {
            // this has to be a valid address to the name string
            const char* name = reinterpret_cast<const char*>(pop());
            Value* value = reinterpret_cast<Value*>(pop());
            env->update(name, value);
            break;
        }
        case OP_PUSH_ENV: {
            if (env) {
                env = std::make_unique<Env>(std::move(env));
            } else {
                std::cerr << "can not push env. env is empty.\n";
            }
            break;
        }
        case OP_POP_ENV: {
            if (env) {
                env = env->take_parent();
            } else {
                std::cerr << "can not pop env. it does not exist.\n";
            }
            break;
        }
        case OP_LOADC: {
            push(read_const_from_progmem());
            break;
        }
        case OP_POP: pop(); break;
        case OP_ADD: BINOP(+); break;
        case OP_SUB: BINOP(-); break;
        case OP_MUL: BINOP(*); break;
        case OP_DIV: BINOP(/); break;
        case OP_REM: BINOP(%); break;
        case OP_LE:  BINOP(<); break;
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
        case OP_EQ: {
            auto rhs = pop();
            auto lhs = pop();
            push(logic_binop(OP_EQ, lhs, rhs));
            break;
        }
        case OP_NOT: {
            auto lhs = pop();
            push(logic_binop(OP_NOT, lhs, 0));
            break;
        }
        case OP_JUMP: {
            auto target = read_const_from_progmem();
            ip = reinterpret_cast<uint8_t*>(target);
            break;
        }
        case OP_JUMPZ: {
            auto condition = pop();
            auto target = read_const_from_progmem();
            if (condition == CARL_FALSE) {
                ip = reinterpret_cast<uint8_t*>(target);
            }
            break;
        }
        case OP_HALT: return STEP_HALT;
        default:
            std::cerr << "OpCode " << op << " not implemented in VM.\n";
            return STEP_ERROR;
    }
    return STEP_OK;
#undef BINOP
}

InterpretResult VM::run(bool print_trace_flag) {
    InterpretResult r = STEP_OK; // empty program is okay
    do {
        r = step(print_trace_flag);
        if (print_trace_flag) print_stack();
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

carl_stackelem_t VM::peek() {
    carl_stackelem_t v = *sp;
    return v;
}

carl_int_t VM::read_const_from_progmem() {
    carl_int_t* vp = reinterpret_cast<carl_int_t*>(ip);
    ip += sizeof(carl_int_t);
    return *vp;
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
            return (lb && rb) ? CARL_TRUE : CARL_FALSE;
            // return !lb ? lhs : rhs; // if lhs is true, return rhs for (false, true, nil).
        case OP_OR:
            if (lb) return CARL_TRUE;
            if (!lb && (rhs == CARL_NIL)) return CARL_NIL;
            return (lb || rb) ? CARL_TRUE : CARL_FALSE;
            // if (lb) return lhs;
            // return rhs; // always return rhs for (false, true, nil).
        case OP_NOT:
            return lhs == CARL_NIL ? lhs : static_cast<carl_int_t>(!lb);
        case OP_EQ:
            return lhs == rhs ? CARL_TRUE : CARL_FALSE;
        default:
            std::cerr << "unsupported logical_binop" << std::endl;
    }
    return 0;
}

carl_stackelem_t VM::mkbasic() {
    // this new is never cleaned up atm
    BasicValue* v = new BasicValue(); 
    v->type = ValueType::Basic;
    v->value = pop();
    return reinterpret_cast<carl_stackelem_t>(v);
}

carl_stackelem_t VM::gtbasic() {
    BasicValue* v = reinterpret_cast<BasicValue*>(pop());
    if (v->type != ValueType::Basic) {
        std::cerr << "value in gtbasic is not of type Basic!\n";
        return 0;
    }
    return v->value;
}

void VM::update_value() {
    // this operation just overwrites target with val
    auto target_addr = reinterpret_cast<Value*>(pop());
    auto new_val = reinterpret_cast<Value*>(peek());
    if (target_addr->type != new_val->type) {
        std::cerr << "target and new value are not of the same type. aborting assign.\n";
        return;
    }
    // TODO: make sure this will also work for functions and other complex values.
    memcpy((void*)target_addr, (void*)new_val, get_size(target_addr->type));
}
