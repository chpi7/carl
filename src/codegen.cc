#include "carl/codegen.h"

#include <unordered_map>

#include "carl/common.h"

using namespace carl;

static const std::unordered_map<TokenType, OpCode> opMap = {
    {TOKEN_PLUS, OP_ADD},  {TOKEN_MINUS, OP_SUB}, {TOKEN_STAR, OP_MUL},
    {TOKEN_SLASH, OP_DIV}, {TOKEN_BANG, OP_NEG},
};

static const std::unordered_map<TokenType, OpCode> unaryOpMap = {
    {TOKEN_MINUS, OP_NEG} , {TOKEN_BANG, OP_NEG},
};

CodeGenerator::CodeGenerator() {
    chunk = std::make_unique<Chunk>(2048);
}

Chunk* CodeGenerator::get_chunk() {
    return chunk.get(); 
}

std::unique_ptr<Chunk> CodeGenerator::take_chunk() {
    return std::move(chunk);
}

void CodeGenerator::visit_invalid(Invalid* invalid) {
    std::cerr << "code generator visiting invalid node is not allowed."
              << std::endl;
    return;
}

void CodeGenerator::visit_binary(Binary* binary) {
    binary->get_lhs()->accept(this);
    binary->get_rhs()->accept(this);
    auto tokenType = binary->get_op().type;
    if (!opMap.contains(tokenType)) {
        std::cerr << "operator token type not found in opMap" << std::endl;
        return;
    }
    OpCode opCode = opMap.at(tokenType);
    chunk->write_byte(opCode);
}

void CodeGenerator::visit_unary(Unary* unary) {
    unary->get_operand()->accept(this);
    auto tokenType = unary->get_op().type;
    if (!unaryOpMap.contains(tokenType)) {
        std::cerr << "operator token type not found in opMap" << std::endl;
        return;
    }
    OpCode opCode = unaryOpMap.at(tokenType);
    chunk->write_byte(opCode);
}

void CodeGenerator::visit_variable(Variable* variable){}

void CodeGenerator::visit_literal(Literal* literal){}

void CodeGenerator::visit_string(String* string){}

void CodeGenerator::visit_number(Number* number) {
    // TODO maybe do this more nicely?
    carl_int_t v = atoi(number->get_value().start);
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(v);
}

void CodeGenerator::visit_exprstmt(ExprStmt* exprstmt){}

void CodeGenerator::visit_letstmt(LetStmt* letstmt){}
