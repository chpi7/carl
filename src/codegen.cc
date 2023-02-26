#include "carl/codegen.h"

#include <unordered_map>
#include <type_traits>

#include "carl/common.h"

#include <cstring>

using namespace carl;

static const std::unordered_map<TokenType, OpCode> opMap = {
    {TOKEN_PLUS, OP_ADD},  {TOKEN_MINUS, OP_SUB}, {TOKEN_STAR, OP_MUL},
    {TOKEN_SLASH, OP_DIV}, {TOKEN_BANG, OP_NEG}, {TOKEN_PERC, OP_REM},
    {TOKEN_AND, OP_AND}, {TOKEN_OR, OP_OR}, {TOKEN_EQUAL_EQUAL, OP_EQ},
    {TOKEN_LESS, OP_LE},
};

static const std::unordered_map<TokenType, OpCode> unaryOpMap = {
    {TOKEN_MINUS, OP_NEG} , {TOKEN_BANG, OP_NOT},
};

CodeGenerator::CodeGenerator() {
    chunk = std::make_unique<Chunk>(2048);
}

void CodeGenerator::generate(std::vector<std::shared_ptr<AstNode>>& nodes) {
    for (auto& node : nodes) {
        node->accept(this);
    }
    chunk->write_byte(OP_HALT);
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
    chunk->write_byte(OP_GTBASIC);

    binary->get_rhs()->accept(this);
    chunk->write_byte(OP_GTBASIC);
    auto tokenType = binary->get_op().type;
    if (!opMap.contains(tokenType)) {
        std::cerr << "operator token type not found in opMap" << std::endl;
        return;
    }
    OpCode opCode = opMap.at(tokenType);
    chunk->write_byte(opCode);
    chunk->write_byte(OP_MKBASIC);
}

void CodeGenerator::visit_assignment(Assignment* assignment) {
    // generate the value to assign
    assignment->get_expr()->accept(this);

    // generate the heap address of the assignment target.
    assignment->get_target()->accept(this);

    // assume that by assigning the type does not change
    chunk->write_byte(OP_UPDATE_VALUE);
}

void CodeGenerator::visit_unary(Unary* unary) {
    unary->get_operand()->accept(this);
    chunk->write_byte(OP_GTBASIC);
    auto tokenType = unary->get_op().type;
    if (!unaryOpMap.contains(tokenType)) {
        std::cerr << "operator token type not found in opMap" << std::endl;
        return;
    }
    OpCode opCode = unaryOpMap.at(tokenType);
    chunk->write_byte(opCode);
    chunk->write_byte(OP_MKBASIC);
}

void CodeGenerator::visit_variable(Variable* variable){
    // retrieve the stored name for this name
    auto name = chunk->save_name(std::string(variable->get_name().start, variable->get_name().length));
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(reinterpret_cast<carl_int_t>(name->c_str()));
    chunk->write_byte(OP_GET_VAR);
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
        chunk->write_byte(OP_NIL);
    }
    chunk->write_byte(OP_MKBASIC);
#undef IS_TOK
}

void CodeGenerator::visit_string(String*){
    std::cerr << "String codegen not implemented yet!" << std::endl;
    // MK_STRING or smth like that.
}

void CodeGenerator::visit_number(Number* number) {
    carl_int_t v = atoi(number->get_value().start);
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(v);
    chunk->write_byte(OP_MKBASIC);
}

void CodeGenerator::visit_exprstmt(ExprStmt* exprstmt){
    exprstmt->get_expr()->accept(this);
    chunk->write_byte(OP_POP);
}

void CodeGenerator::visit_letstmt(LetStmt* letstmt){
    // compute initializer value
    letstmt->get_initializer()->accept(this);

    // copy name string and save to chunk
    auto name = chunk->save_name(std::string(letstmt->get_name().start, letstmt->get_name().length));
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(reinterpret_cast<carl_int_t>(name->c_str()));
    chunk->write_byte(OP_DEFINE_VAR);
}

void CodeGenerator::visit_block(Block* block) {
    chunk->write_byte(OP_PUSH_ENV);
    for (auto& decl : block->get_declarations()) {
        decl->accept(this);
    }
    chunk->write_byte(OP_POP_ENV);
}

void CodeGenerator::visit_whilestmt(WhileStmt* whilestmt) {
    // evaluate the condition
    uint8_t* cond_loc = chunk->get_memory() + chunk->get_write_offset();
    whilestmt->get_condition()->accept(this);

    // jump over body if false
    chunk->write_byte(OP_GTBASIC);
    chunk->write_byte(OP_JUMPZ);
    uint8_t* loc = chunk->write_int_const(0);

    whilestmt->get_body()->accept(this);

    chunk->write_byte(OP_JUMP);
    chunk->write_int_const(reinterpret_cast<carl_int_t>(cond_loc));

    chunk->patch_const(loc, reinterpret_cast<uint64_t>(chunk->get_memory() + chunk->get_write_offset()));
}
