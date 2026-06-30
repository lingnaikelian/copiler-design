#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_LEN 256

typedef enum {
    TOKEN_ID,
    TOKEN_TYPE,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_SEMI,
    TOKEN_COMMA,
    TOKEN_ASSIGNOP,
    TOKEN_RELOP,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_DIV,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_DOT,
    TOKEN_NOT,
    TOKEN_LP,
    TOKEN_RP,
    TOKEN_LB,
    TOKEN_RB,
    TOKEN_LC,
    TOKEN_RC,
    TOKEN_STRUCT,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    int line;
    int column;
    char value[MAX_TOKEN_LEN];
    int intVal;
    float floatVal;
} Token;

extern Token currentToken;
extern int lineNumber;
extern int columnNumber;
extern FILE* lex_in;

void initLexer(FILE* input);
Token getNextToken();
void printToken(Token token);

#endif
