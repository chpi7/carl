#include <iostream>

#include "carl/common.h"

using namespace carl;

static void repl() {
    char line[1024];
    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        // TODO do something with the line
    }
}

int main(int argc, char* argv[]) {
    std::cout << "carl (version " << CARL_VERSION << ")" << std::endl;
    repl();
}
