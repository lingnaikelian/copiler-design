#include <stdio.h>
#include <stdlib.h>
#include "../parser/node.h"
#include "../scanner/common.h"
#include "semantic.h"

extern int yyparse();
extern ASTNode* programRoot;
extern int errorCount;

int main(int argc, char** argv) {
    if (argc > 1) {
        lex_in = fopen(argv[1], "r");
        if (!lex_in) {
            fprintf(stderr, "Cannot open file: %s\n", argv[1]);
            return 1;
        }
    }
    
    initLexer(lex_in);
    
    if (yyparse() == 0 && errorCount == 0) {
        semanticAnalysis(programRoot);
    }
    
    freeAST(programRoot);
    
    if (argc > 1) {
        fclose(lex_in);
    }
    
    return 0;
}