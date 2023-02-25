#include <iostream>

#include "carl/common.h"
#include "carl/parser.h"
#include "carl/codegen.h"
#include "carl/vm/vm.h"
#include "carl/ast/ast_printer.h"

using namespace carl;

static void interpret(const char* line) {
    auto scanner = std::make_shared<Scanner>();
    Parser parser;
    CodeGenerator gen;
#ifdef DEBUG
    AstPrinter v(std::cout);
#endif
    VM vm(512);

    scanner->init(line);
    parser.set_scanner(scanner);

    auto expr = parser.statement();
    gen.generate(expr);
#ifdef DEBUG
    expr->accept(&v);
    std::cout << std::endl;
    gen.get_chunk()->print(std::cout);
#endif

    vm.load_chunk(gen.take_chunk());
    vm.run();

#ifdef DEBUG
    printf("Stack top hex: %016lx\n", vm.get_stack_top());
    printf("Stack top dec: %ld\n", vm.get_stack_top());
#endif
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