#include <iostream>

#include "carl/common.h"
#include "carl/parser.h"
#include "carl/llvm/codegen.h"
#include "carl/llvm/jit.h"
#include "carl/ast/ast_printer.h"

using namespace carl;
LLJITWrapper jit;

static void interpret(const char* line) {
    auto scanner = std::make_shared<Scanner>();
    Parser parser;
    LLVMCodeGenerator cg;
#ifdef DEBUG
    AstPrinter v(std::cout);
#endif

    scanner->init(line);
    parser.set_scanner(scanner);
    auto decls = parser.parse();
    // TODO: change this to check for the actual type...
    ExprStmt* e = reinterpret_cast<ExprStmt*>(decls.front().get());

    cg.generate_eval(e->get_expr());
    auto mod = cg.take_module();

    auto tracker = jit.load_module(mod);
    auto s = jit.lookup("__expr_wrapper");
    if (!s) {
        std::cerr << "could not find __expr_wrapper symbol in jit." << std::endl;
        return;
    }

    auto wrapper = reinterpret_cast<int64_t (*)()>(s.value());
    int64_t result = wrapper();
    std::cout << result << std::endl;
    // code did execute -> value will be there
    if(tracker.value()->remove()) {
        std::cerr << "could not unload wrapper module" << std::endl;
    }
}

static void repl() {
    char line[1024];
    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

int main(int argc, char* argv[]) {
    std::cout << "carl (version " << CARL_VERSION << ")" << std::endl;
    repl();
}