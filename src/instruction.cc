#include "carl/vm/instruction.h"

using namespace carl;

Chunk::Chunk(uint64_t size) {
    memory = new uint8_t[size];
    write_pos = memory;
}

Chunk::~Chunk() {
    if (memory != nullptr) {
        delete[] memory;
        memory = nullptr;
        write_pos = nullptr;
    }
}

void Chunk::write_opcode(OpCode opcode) { *(write_pos++) = (uint8_t)opcode; }

void Chunk::write_int_const(carl_int_t c) {
    *((carl_int_t*)write_pos) = c;
    write_pos += sizeof(carl_int_t);
}

static void print_simple_instruction(std::ostream& os, const char* name,
                                     uint8_t** pos) {
    os << name;
    *pos += 1;
}

static void print_single_arg_instruction(std::ostream& os, const char* name,
                                         uint8_t** pos) {
    os << name << "\t" << (carl_int_t)((*pos) + 1);
    *pos += sizeof(carl_int_t) + 1;
}

void Chunk::print(std::ostream& os) {
    uint8_t* pos = memory;
    while (pos <= memory + size && pos < write_pos) {
        auto instruction = *pos;
        char fmtbuf[128];
        sprintf(fmtbuf, "%4x: ", (uint32_t)(pos - memory));
        os << fmtbuf;
        switch (instruction) {
            case OP_NOP:
                print_simple_instruction(os, "OP_NOP", &pos);
                break;
            case OP_MKBASIC:
                print_simple_instruction(os, "OP_MKBASIC", &pos);
                break;
            case OP_GTBASIC:
                print_simple_instruction(os, "OP_GTBASIC", &pos);
                break;
            case OP_LOADC:
                print_single_arg_instruction(os, "OP_LOADC", &pos);
                break;
            case OP_POP:
                print_simple_instruction(os, "OP_POP", &pos);
                break;
            case OP_ADD:
                print_simple_instruction(os, "OP_ADD", &pos);
                break;
            case OP_SUB:
                print_simple_instruction(os, "OP_SUB", &pos);
                break;
            case OP_MUL:
                print_simple_instruction(os, "OP_MUL", &pos);
                break;
            case OP_NEG:
                print_simple_instruction(os, "OP_NEG", &pos);
                break;
            case OP_EQ:
                print_simple_instruction(os, "OP_EQ", &pos);
                break;
            case OP_LE:
                print_simple_instruction(os, "OP_LE", &pos);
                break;
            case OP_LEQ:
                print_simple_instruction(os, "OP_LEQ", &pos);
                break;
            case OP_JUMP:
                print_simple_instruction(os, "OP_JUMP", &pos);
                break;
            case OP_JUMPZ:
                print_simple_instruction(os, "OP_JUMPZ", &pos);
                break;
            case OP_HALT:
                print_simple_instruction(os, "OP_HALT", &pos);
                break;
            default:
                os << "<unknown " << instruction << ">";
                pos += 1;
        }
        os << std::endl;
    }
}

uint64_t Chunk::get_write_offset() { return write_pos - memory; }
