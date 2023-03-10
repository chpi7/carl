#include "carl/vm/chunk.h"

#include <cstring>
#include <iostream>

#include "carl/common.h"

using namespace carl;

Chunk::Chunk(uint64_t size) : size(size) {
    memory = new uint8_t[size];
    memset(memory, 0, size);
    write_pos = memory;
}

Chunk::~Chunk() {
    if (memory != nullptr) {
        delete[] memory;
        memory = nullptr;
        write_pos = nullptr;
    }
}

uint8_t* Chunk::write_byte(OpCode opcode) { 
    assert(get_write_offset() + 1 < size && "No more space in chunk for opcode");
    auto retval = write_pos;
    *(write_pos) = (uint8_t)opcode; 
    write_pos++;
    return retval;
}

uint8_t* Chunk::write_int_const(carl_int_t c) {
    assert(get_write_offset() + sizeof(carl_int_t) < size && "No more space in chunk for immediate");
    carl_int_t* loc = reinterpret_cast<carl_int_t*>(write_pos);
    *loc = c;
    assert(*loc == c && "Readback incorrect!");

    auto retval = write_pos;
    write_pos += sizeof(carl_int_t);
    return retval;
}

void Chunk::patch_const(uint8_t* address, uint64_t c) {
    carl_int_t* loc = reinterpret_cast<carl_int_t*>(address);
    *loc = c;
    assert(*loc == c && "Readback incorrect!");
}

static void print_simple_instruction(std::ostream& os, const char* name,
                                     uint8_t** pos) {
    char fmtbuf[128]{0};
    sprintf(fmtbuf, "%s", name);
    os << fmtbuf;
    *pos += 1;
}

static void print_single_arg_instruction(std::ostream& os, const char* name,
                                         uint8_t** pos) {
    carl_int_t* val_loc = reinterpret_cast<carl_int_t*>((*pos)+1);
    carl_int_t val = *val_loc;

    char fmtbuf[128]{0};
    sprintf(fmtbuf, "%s %016lx", name, val);
    os << fmtbuf;
    *pos += sizeof(carl_int_t) + 1;
}

static void print_jump_instruction(std::ostream& os, const char* name,
                                         uint8_t** pos, uint8_t* base) {
    carl_int_t* val_loc = reinterpret_cast<carl_int_t*>((*pos)+1);
    carl_int_t val = *val_loc;

    char fmtbuf[128]{0};
    sprintf(fmtbuf, "%s %016lx", name, reinterpret_cast<uint8_t*>(val) - base);
    os << fmtbuf;
    *pos += sizeof(carl_int_t) + 1;
}

int Chunk::print_single(std::ostream& os, uint8_t* pos) {
        uint8_t* initial_pos = pos;
        auto instruction = *pos;
        char fmtbuf[128]{0};
        sprintf(fmtbuf, "%4x: ", (uint32_t)(pos - memory));
        os << fmtbuf;
        switch (instruction) {
            case OP_NOP:
                print_simple_instruction(os, "NOP", &pos);
                break;
            case OP_MKBASIC:
                print_simple_instruction(os, "MKBASIC", &pos);
                break;
            case OP_GTBASIC:
                print_simple_instruction(os, "GTBASIC", &pos);
                break;
            case OP_UPDATE_VALUE:
                print_simple_instruction(os, "OP_UPDATE_VALUE", &pos);
                break;
            case OP_LOADC:
                print_single_arg_instruction(os, "LOADC", &pos);
                break;
            case OP_POP:
                print_simple_instruction(os, "POP", &pos);
                break;
            case OP_DUP:
                print_simple_instruction(os, "DUP", &pos);
                break;
            case OP_ADD:
                print_simple_instruction(os, "ADD", &pos);
                break;
            case OP_SUB:
                print_simple_instruction(os, "SUB", &pos);
                break;
            case OP_MUL:
                print_simple_instruction(os, "MUL", &pos);
                break;
            case OP_DIV:
                print_simple_instruction(os, "DIV", &pos);
                break;
            case OP_REM:
                print_simple_instruction(os, "REM", &pos);
                break;
            case OP_NEG:
                print_simple_instruction(os, "NEG", &pos);
                break;
            case OP_EQ:
                print_simple_instruction(os, "EQ", &pos);
                break;
            case OP_AND:
                print_simple_instruction(os, "AND", &pos);
                break;
            case OP_LE:
                print_simple_instruction(os, "LE", &pos);
                break;
            case OP_LEQ:
                print_simple_instruction(os, "LEQ", &pos);
                break;
            case OP_JUMP:
                print_jump_instruction(os, "JUMP", &pos, get_memory());
                break;
            case OP_JUMPZ:
                print_jump_instruction(os, "JUMPZ", &pos, get_memory());
                break;
            case OP_DEFINE_VAR:
                print_simple_instruction(os, "DEFINE_VAR", &pos);
                break;
            case OP_GET_VAR:
                print_simple_instruction(os, "GET_VAR", &pos);
                break;
            case OP_SET_VAR:
                print_simple_instruction(os, "SET_VAR", &pos);
                break;
            case OP_PUSH_ENV:
                print_simple_instruction(os, "PUSH_ENV", &pos);
                break;
            case OP_POP_ENV:
                print_simple_instruction(os, "POP_ENV", &pos);
                break;
            case OP_HALT:
                print_simple_instruction(os, "HALT", &pos);
                break;
            default:
                os << "<unknown " << instruction << ">";
                pos += 1;
        }

    return pos - initial_pos;
}

void Chunk::print(std::ostream& os) {
    uint8_t* pos = memory;
    while (pos <= memory + size && pos < write_pos) {
        pos += print_single(os, pos);
        os << std::endl;
    }
}

uint8_t* Chunk::get_memory() {
    return memory;
}

uint64_t Chunk::get_write_offset() { return write_pos - memory; }

std::shared_ptr<std::string> Chunk::get_name(std::string name) {
    if (!names.contains(name)) {
        std::cerr << "name " << name << " not found.\n";
        return std::make_shared<std::string>();
    }
    return names[name];
}

std::shared_ptr<std::string> Chunk::save_name(std::string name) {
    if (names.contains(name)) return names[name];
    names[name] = std::make_shared<std::string>(name);
    return names[name];
}

std::unordered_map<std::string, std::shared_ptr<std::string>>& Chunk::get_names() {
    return names;
}
