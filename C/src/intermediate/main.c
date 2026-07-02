#include <stdio.h>
#include <stdlib.h>
#include "../parser/node.h"
#include "../parser/parser.tab.h"
#include "../scanner/common.h"
#include "../semantic/semantic.h"
#include "ir.h"

extern FILE* lex_in;
extern int errorCount;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    
    lex_in = fopen(argv[1], "r");
    if (lex_in == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);
        return 1;
    }
    
    extern ASTNode* programRoot;
    programRoot = NULL;
    yyparse();
    
    fclose(lex_in);
    
    if (programRoot != NULL) {
        semanticAnalysis(programRoot);
        
        if (errorCount == 0) {
            genIR(programRoot);
            printIR(stdout);
        }
        
        freeAST(programRoot);
        freeIR();
    }
    
    return 0;
}
