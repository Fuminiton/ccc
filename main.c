#include "ccc.h"


int main(int argc, char **argv) {
    if (argc != 2) {
        error("%s: Invalid number of arguments", argv[0]);
    }

    // Tokenize
    user_input = argv[1];
    token = tokenize();

    // Parse
    program();

    // Generate
    gen();

    return 0;
}
