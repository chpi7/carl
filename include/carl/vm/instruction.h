#ifndef carl_instruction_h
#define carl_instruction_h

namespace carl {
enum OpCode {
    OP_NOP,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,

    OP_MKBASIC,
    OP_GTBASIC,
    OP_LOADC,
    OP_POP,
    OP_DUP,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_REM,
    OP_NEG,
    OP_EQ,
    OP_LE,
    OP_LEQ,
    OP_JUMP,
    OP_JUMPZ,
    OP_DEFINE_VAR,
    OP_GET_VAR,
    OP_SET_VAR,
    OP_PUSH_ENV,
    OP_POP_ENV,
    OP_HALT,
};
}

#endif