%{
#include "common.h"

Token currentToken;
int lineNumber = 1;
int columnNumber = 1;

#define RETURN_TOKEN(t) \
    currentToken.type = t; \
    currentToken.line = lineNumber; \
    currentToken.column = columnNumber - yyleng; \
    strncpy(currentToken.value, yytext, MAX_TOKEN_LEN - 1); \
    currentToken.value[MAX_TOKEN_LEN - 1] = '\0'; \
    return currentToken;

#define RETURN_TOKEN_NO_VALUE(t) \
    currentToken.type = t; \
    currentToken.line = lineNumber; \
    currentToken.column = columnNumber - yyleng; \
    currentToken.value[0] = '\0'; \
    return currentToken;

%}

letter      [a-zA-Z_]
digit       [0-9]
ident       {letter}({letter}|{digit})*
integer     {digit}+
floatnum    {digit}+\.{digit}+
charconst   \'[^']\'

ws          [ \t\r]
nl          \n

%%

{ws}+       { columnNumber += yyleng; }
{nl}        { lineNumber++; columnNumber = 1; }

"/*"        {
                int c;
                while ((c = input()) != EOF) {
                    if (c == '*') {
                        if ((c = input()) == '/') break;
                        unput(c);
                    }
                    if (c == '\n') {
                        lineNumber++;
                        columnNumber = 1;
                    } else {
                        columnNumber++;
                    }
                }
            }

"//"[^\n]*  { columnNumber += yyleng; }

"int"       { RETURN_TOKEN_NO_VALUE(TOKEN_INT); }
"char"      { RETURN_TOKEN_NO_VALUE(TOKEN_CHAR); }
"float"     { RETURN_TOKEN_NO_VALUE(TOKEN_FLOAT); }
"void"      { RETURN_TOKEN_NO_VALUE(TOKEN_VOID); }

"if"        { RETURN_TOKEN_NO_VALUE(TOKEN_IF); }
"else"      { RETURN_TOKEN_NO_VALUE(TOKEN_ELSE); }
"while"     { RETURN_TOKEN_NO_VALUE(TOKEN_WHILE); }
"for"       { RETURN_TOKEN_NO_VALUE(TOKEN_FOR); }
"return"    { RETURN_TOKEN_NO_VALUE(TOKEN_RETURN); }
"break"     { RETURN_TOKEN_NO_VALUE(TOKEN_BREAK); }
"continue"  { RETURN_TOKEN_NO_VALUE(TOKEN_CONTINUE); }

"read"      { RETURN_TOKEN_NO_VALUE(TOKEN_READ); }
"write"     { RETURN_TOKEN_NO_VALUE(TOKEN_WRITE); }

"+"         { RETURN_TOKEN_NO_VALUE(TOKEN_PLUS); }
"-"         { RETURN_TOKEN_NO_VALUE(TOKEN_MINUS); }
"*"         { RETURN_TOKEN_NO_VALUE(TOKEN_TIMES); }
"/"         { RETURN_TOKEN_NO_VALUE(TOKEN_DIV); }
"%"         { RETURN_TOKEN_NO_VALUE(TOKEN_MOD); }

"="         { RETURN_TOKEN_NO_VALUE(TOKEN_ASSIGN); }
"=="        { RETURN_TOKEN_NO_VALUE(TOKEN_EQUAL); }
"!="        { RETURN_TOKEN_NO_VALUE(TOKEN_NOTEQUAL); }
"<"         { RETURN_TOKEN_NO_VALUE(TOKEN_LESS); }
">"         { RETURN_TOKEN_NO_VALUE(TOKEN_GREATER); }
"<="        { RETURN_TOKEN_NO_VALUE(TOKEN_LEQ); }
">="        { RETURN_TOKEN_NO_VALUE(TOKEN_GEQ); }

"&&"        { RETURN_TOKEN_NO_VALUE(TOKEN_AND); }
"||"        { RETURN_TOKEN_NO_VALUE(TOKEN_OR); }
"!"         { RETURN_TOKEN_NO_VALUE(TOKEN_NOT); }

"("         { RETURN_TOKEN_NO_VALUE(TOKEN_LPAREN); }
")"         { RETURN_TOKEN_NO_VALUE(TOKEN_RPAREN); }
"{"         { RETURN_TOKEN_NO_VALUE(TOKEN_LBRACE); }
"}"         { RETURN_TOKEN_NO_VALUE(TOKEN_RBRACE); }
"["         { RETURN_TOKEN_NO_VALUE(TOKEN_LBRACK); }
"]"         { RETURN_TOKEN_NO_VALUE(TOKEN_RBRACK); }
","         { RETURN_TOKEN_NO_VALUE(TOKEN_COMMA); }
";"         { RETURN_TOKEN_NO_VALUE(TOKEN_SEMICOLON); }
":"         { RETURN_TOKEN_NO_VALUE(TOKEN_COLON); }
"."         { RETURN_TOKEN_NO_VALUE(TOKEN_DOT); }

{ident}     { RETURN_TOKEN(TOKEN_IDENT); }
{integer}   { RETURN_TOKEN(TOKEN_INTCON); }
{floatnum}  { RETURN_TOKEN(TOKEN_FLOATCON); }
{charconst} { RETURN_TOKEN(TOKEN_CHARCON); }

.           { 
                fprintf(stderr, "Lexical error at line %d, column %d: unexpected character '%c'\n", 
                        lineNumber, columnNumber, yytext[0]);
                columnNumber += yyleng;
            }

<<EOF>>     { RETURN_TOKEN_NO_VALUE(TOKEN_EOF); }

%%

void initLexer(FILE* input) {
    yyin = input;
    lineNumber = 1;
    columnNumber = 1;
}

Token getNextToken() {
    columnNumber += yyleng;
    return yylex();
}

void printToken(Token token) {
    printf("(");
    switch (token.type) {
        case TOKEN_IDENT:       printf("IDN"); break;
        case TOKEN_INTCON:      printf("INT"); break;
        case TOKEN_CHARCON:     printf("CHA"); break;
        case TOKEN_FLOATCON:    printf("FLT"); break;
        case TOKEN_PLUS:        printf("ADD"); break;
        case TOKEN_MINUS:       printf("SUB"); break;
        case TOKEN_TIMES:       printf("MUL"); break;
        case TOKEN_DIV:         printf("DIV"); break;
        case TOKEN_MOD:         printf("MOD"); break;
        case TOKEN_ASSIGN:      printf("ASS"); break;
        case TOKEN_EQUAL:       printf("EQL"); break;
        case TOKEN_NOTEQUAL:    printf("NEQ"); break;
        case TOKEN_LESS:        printf("LES"); break;
        case TOKEN_GREATER:     printf("GTR"); break;
        case TOKEN_LEQ:         printf("LEQ"); break;
        case TOKEN_GEQ:         printf("GEQ"); break;
        case TOKEN_AND:         printf("AND"); break;
        case TOKEN_OR:          printf("OR"); break;
        case TOKEN_NOT:         printf("NOT"); break;
        case TOKEN_LPAREN:      printf("LPA"); break;
        case TOKEN_RPAREN:      printf("RPA"); break;
        case TOKEN_LBRACE:      printf("LBR"); break;
        case TOKEN_RBRACE:      printf("RBR"); break;
        case TOKEN_LBRACK:      printf("LBS"); break;
        case TOKEN_RBRACK:      printf("RBS"); break;
        case TOKEN_COMMA:       printf("COM"); break;
        case TOKEN_SEMICOLON:   printf("SEM"); break;
        case TOKEN_COLON:       printf("COL"); break;
        case TOKEN_DOT:         printf("DOT"); break;
        case TOKEN_INT:         printf("INT"); break;
        case TOKEN_CHAR:        printf("CHA"); break;
        case TOKEN_FLOAT:       printf("FLT"); break;
        case TOKEN_VOID:        printf("VID"); break;
        case TOKEN_IF:          printf("IF"); break;
        case TOKEN_ELSE:        printf("ELSE"); break;
        case TOKEN_WHILE:       printf("WHILE"); break;
        case TOKEN_FOR:         printf("FOR"); break;
        case TOKEN_RETURN:      printf("RET"); break;
        case TOKEN_BREAK:       printf("BRK"); break;
        case TOKEN_CONTINUE:    printf("CONT"); break;
        case TOKEN_READ:        printf("READ"); break;
        case TOKEN_WRITE:       printf("WRITE"); break;
        case TOKEN_EOF:         printf("EOF"); break;
        default:                printf("UNK"); break;
    }
    
    if (token.value[0] != '\0') {
        printf(",\"%s\"", token.value);
    }
    
    printf(")\n");
}

int yywrap() {
    return 1;
}
