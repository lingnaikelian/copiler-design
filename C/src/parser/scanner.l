%{
#include "node.h"
#include "parser.tab.h"
#include "../scanner/common.h"

extern int errorCount;
extern Token currentToken;
extern int lineNumber;
%}

%option noyywrap

%%

<<EOF>> { 
    return 0; 
}

. { 
    fprintf(stderr, "Error type A at Line %d: Mysterious character\n", lineNumber);
    errorCount++;
    return yylex();
}

%%

int yylex() {
    static int firstCall = 1;
    if (firstCall) {
        firstCall = 0;
    }
    
    getNextToken();
    switch (currentToken.type) {
        case TOKEN_ID:
            yylval.strVal = strdup(currentToken.value);
            return ID;
        case TOKEN_TYPE:
            yylval.strVal = strdup(currentToken.value);
            return TYPE;
        case TOKEN_INT:
            yylval.intVal = currentToken.intVal;
            return INT;
        case TOKEN_FLOAT:
            yylval.floatVal = currentToken.floatVal;
            return FLOAT;
        case TOKEN_SEMI: return SEMI;
        case TOKEN_COMMA: return COMMA;
        case TOKEN_ASSIGNOP: return ASSIGNOP;
        case TOKEN_RELOP: return RELOP;
        case TOKEN_PLUS: return PLUS;
        case TOKEN_MINUS: return MINUS;
        case TOKEN_STAR: return STAR;
        case TOKEN_DIV: return DIV;
        case TOKEN_AND: return AND;
        case TOKEN_OR: return OR;
        case TOKEN_DOT: return DOT;
        case TOKEN_NOT: return NOT;
        case TOKEN_LP: return LP;
        case TOKEN_RP: return RP;
        case TOKEN_LB: return LB;
        case TOKEN_RB: return RB;
        case TOKEN_LC: return LC;
        case TOKEN_RC: return RC;
        case TOKEN_STRUCT: return STRUCT;
        case TOKEN_RETURN: return RETURN;
        case TOKEN_IF: return IF;
        case TOKEN_ELSE: return ELSE;
        case TOKEN_WHILE: return WHILE;
        case TOKEN_EOF: return 0;
        default:
            fprintf(stderr, "Error type A at Line %d: Mysterious character\n", lineNumber);
            errorCount++;
            return yylex();
    }
}
