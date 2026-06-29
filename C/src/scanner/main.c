#include "common.h"

int main(int argc, char* argv[]) {
    FILE* input;
    
    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (!input) {
            fprintf(stderr, "Error: cannot open file %s\n", argv[1]);
            return 1;
        }
    } else {
        input = stdin;
        printf("Enter C-- source code (Ctrl+D to end):\n");
    }
    
    initLexer(input);
    
    Token token;
    
    do {
        token = getNextToken();
        printToken(token);
    } while (token.type != TOKEN_EOF);
    
    if (input != stdin) {
        fclose(input);
    }
    
    return 0;
}
