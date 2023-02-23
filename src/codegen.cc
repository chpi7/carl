#include "carl/codegen.h"

#include <unordered_map>

#include "carl/common.h"

#include <cstring>

using namespace carl;

static const std::unordered_map<TokenType, OpCode> opMap = {
    {TOKEN_PLUS, OP_ADD},  {TOKEN_MINUS, OP_SUB}, {TOKEN_STAR, OP_MUL},
    {TOKEN_SLASH, OP_DIV}, {TOKEN_BANG, OP_NEG}, {TOKEN_PERC, OP_REM},
    {TOKEN_AND, OP_AND}, {TOKEN_OR, OP_OR},
};

static const std::unordered_map<TokenType, OpCode> unaryOpMap = {
    {TOKEN_MINUS, OP_NEG} , {TOKEN_BANG, OP_NOT},
};

CodeGenerator::CodeGenerator() {
    chunk = std::make_unique<Chunk>(2048);
}

void CodeGenerator::generate(std::shared_ptr<AstNode> node) {
    node->accept(this);
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

void CodeGenerator::visit_variable(Variable*){
    std::cerr << "Variable codegen not implemented yet!" << std::endl;
}

void CodeGenerator::visit_literal(Literal* literal){
    auto tok = literal->get_value();
#define IS_TOK(s, l) (tok.length == l && strncmp(tok.start, s, l) == 0)
    if (IS_TOK("true", 4)) {
        chunk->write_byte(OP_TRUE);
    } else if (IS_TOK("false", 5)) {
        chunk->write_byte(OP_FALSE);
    } else if (IS_TOK("nil", 3)) {
        chunk->write_byte(OP_NIL);
    } else {
        std::cerr << "Unkown literal: " << std::string(tok.start, tok.length) << std::endl;
    }
#undef IS_TOK
}

void CodeGenerator::visit_string(String*){
    std::cerr << "String codegen not implemented yet!" << std::endl;
}

void CodeGenerator::visit_number(Number* number) {
    carl_int_t v = atoi(number->get_value().start);
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(v);
}

void CodeGenerator::visit_exprstmt(ExprStmt* exprstmt){
    exprstmt->accept(this);
    chunk->write_byte(OP_POP);
}

void CodeGenerator::visit_letstmt(LetStmt* letstmt){
    // auto name_tok = letstmt->get_name();
    // uint64_t name_hash = hash_string(name_tok.start, name_tok.length);
    // name_hashes.emplace(std::piecewise_construct, std::forward_as_tuple(name_tok.start, name_tok.length), std::forward_as_tuple(name_hash));

    letstmt->accept(this); // compute initializer value
    chunk->write_byte(OP_MKBASIC); // move to heap
    // chunk->write_int_const(name_hash); // put variable id
    chunk->write_byte(OP_DEFINE_VAR); // link id -> heap object
}
