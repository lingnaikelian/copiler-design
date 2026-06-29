#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKEN_LEN 256
#define MAX_LINE_NUMBER 10000

typedef enum {
    TOKEN_NUL,
    TOKEN_IDENT,
    TOKEN_INTCON,
    TOKEN_CHARCON,
    TOKEN_FLOATCON,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_TIMES,
    TOKEN_DIV,
    TOKEN_MOD,
    TOKEN_ASSIGN,
    TOKEN_EQUAL,
    TOKEN_NOTEQUAL,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_LEQ,
    TOKEN_GEQ,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACK,
    TOKEN_RBRACK,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_DOT,

    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_FLOAT,
    TOKEN_VOID,

    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,

    TOKEN_READ,
    TOKEN_WRITE,

    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    int line;
    int column;
    char value[MAX_TOKEN_LEN];
} Token;

extern Token currentToken;
extern int lineNumber;
extern int columnNumber;
extern FILE* yyin;

void initLexer(FILE* input);
Token getNextToken();
void printToken(Token token);

#endif
