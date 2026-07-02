%{
#include "common.h"

Token currentToken;
int lineNumber = 1;
int columnNumber = 1;
int errorCount = 0;

#define RETURN_TOKEN(t) \
    currentToken.type = t; \
    currentToken.line = lineNumber; \
    currentToken.column = columnNumber - lex_leng; \
    strncpy(currentToken.value, lex_text, MAX_TOKEN_LEN - 1); \
    currentToken.value[MAX_TOKEN_LEN - 1] = '\0'; \
    columnNumber += lex_leng; \
    return t;

#define RETURN_TOKEN_NO_VALUE(t) \
    currentToken.type = t; \
    currentToken.line = lineNumber; \
    currentToken.column = columnNumber - lex_leng; \
    currentToken.value[0] = '\0'; \
    columnNumber += lex_leng; \
    return t;

#define RETURN_TOKEN_INT(t, val) \
    currentToken.type = t; \
    currentToken.line = lineNumber; \
    currentToken.column = columnNumber - lex_leng; \
    currentToken.intVal = val; \
    currentToken.value[0] = '\0'; \
    columnNumber += lex_leng; \
    return t;

#define RETURN_TOKEN_FLOAT(t, val) \
    currentToken.type = t; \
    currentToken.line = lineNumber; \
    currentToken.column = columnNumber - lex_leng; \
    currentToken.floatVal = val; \
    currentToken.value[0] = '\0'; \
    columnNumber += lex_leng; \
    return t;

%}

%option prefix="lex_"

letter      [a-zA-Z_]
digit       [0-9]
octdigit    [0-7]
hexdigit    [0-9a-fA-F]
id          {letter}({letter}|{digit})*
decimal     {digit}+
octal       0{octdigit}+
hexadecimal 0[xX]{hexdigit}+
floatnum    {digit}+\.{digit}+
scientific  ({digit}+\.{digit}*|\.{digit}+|{digit}+)[eE][+-]?{digit}+

ws          [ \t\r]
nl          \n

%x COMMENT

%%

{ws}+       { columnNumber += lex_leng; }
{nl}        { lineNumber++; columnNumber = 1; }

"/*"        { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>\n { lineNumber++; columnNumber = 1; }
<COMMENT>.  { /* skip */ }
<COMMENT><<EOF>> { 
    fprintf(stderr, "Error type A at Line %d: unterminated comment\n", lineNumber);
    errorCount++;
    BEGIN(INITIAL);
}

"//"[^\n]*  { columnNumber += lex_leng; }

"int"       { RETURN_TOKEN(TOKEN_TYPE); }
"float"     { RETURN_TOKEN(TOKEN_TYPE); }

"struct"    { RETURN_TOKEN_NO_VALUE(TOKEN_STRUCT); }
"return"    { RETURN_TOKEN_NO_VALUE(TOKEN_RETURN); }
"if"        { RETURN_TOKEN_NO_VALUE(TOKEN_IF); }
"else"      { RETURN_TOKEN_NO_VALUE(TOKEN_ELSE); }
"while"     { RETURN_TOKEN_NO_VALUE(TOKEN_WHILE); }

"+"         { RETURN_TOKEN_NO_VALUE(TOKEN_PLUS); }
"-"         { RETURN_TOKEN_NO_VALUE(TOKEN_MINUS); }
"*"         { RETURN_TOKEN_NO_VALUE(TOKEN_STAR); }
"/"         { RETURN_TOKEN_NO_VALUE(TOKEN_DIV); }
"<"         { RETURN_TOKEN(TOKEN_RELOP); }
"<="        { RETURN_TOKEN(TOKEN_RELOP); }
">"         { RETURN_TOKEN(TOKEN_RELOP); }
">="        { RETURN_TOKEN(TOKEN_RELOP); }
"=="        { RETURN_TOKEN(TOKEN_RELOP); }
"!="        { RETURN_TOKEN(TOKEN_RELOP); }
"="         { RETURN_TOKEN_NO_VALUE(TOKEN_ASSIGNOP); }
"&&"        { RETURN_TOKEN_NO_VALUE(TOKEN_AND); }
"||"        { RETURN_TOKEN_NO_VALUE(TOKEN_OR); }
"!"         { RETURN_TOKEN_NO_VALUE(TOKEN_NOT); }
"."         { RETURN_TOKEN_NO_VALUE(TOKEN_DOT); }
"("         { RETURN_TOKEN_NO_VALUE(TOKEN_LP); }
")"         { RETURN_TOKEN_NO_VALUE(TOKEN_RP); }
"{"         { RETURN_TOKEN_NO_VALUE(TOKEN_LC); }
"}"         { RETURN_TOKEN_NO_VALUE(TOKEN_RC); }
"["         { RETURN_TOKEN_NO_VALUE(TOKEN_LB); }
"]"         { RETURN_TOKEN_NO_VALUE(TOKEN_RB); }
","         { RETURN_TOKEN_NO_VALUE(TOKEN_COMMA); }
";"         { RETURN_TOKEN_NO_VALUE(TOKEN_SEMI); }

{scientific} { RETURN_TOKEN_FLOAT(TOKEN_FLOAT, atof(lex_text)); }
{floatnum}   { RETURN_TOKEN_FLOAT(TOKEN_FLOAT, atof(lex_text)); }
{hexadecimal} { RETURN_TOKEN_INT(TOKEN_INT, (int)strtol(lex_text, NULL, 0)); }
{octal}      { RETURN_TOKEN_INT(TOKEN_INT, (int)strtol(lex_text, NULL, 0)); }
{decimal}    { RETURN_TOKEN_INT(TOKEN_INT, atoi(lex_text)); }

{id}         { RETURN_TOKEN(TOKEN_ID); }

.            { fprintf(stderr, "Error type A at Line %d: Mysterious character \"%c\"\n", lineNumber, lex_text[0]); errorCount++; columnNumber += lex_leng; }

<<EOF>>      { currentToken.type = TOKEN_EOF; currentToken.line = lineNumber; currentToken.column = columnNumber; currentToken.value[0] = '\0'; return 0; }

%%

void initLexer(FILE* input) {
    lex_in = input;
    lineNumber = 1;
    columnNumber = 1;
}

Token getNextToken() {
    lex_lex();
    return currentToken;
}

void printToken(Token token) {
    printf("(");
    switch (token.type) {
        case TOKEN_ID:           printf("ID"); break;
        case TOKEN_TYPE:         printf("TYPE"); break;
        case TOKEN_INT:          printf("INT"); break;
        case TOKEN_FLOAT:        printf("FLOAT"); break;
        case TOKEN_PLUS:         printf("PLUS"); break;
        case TOKEN_MINUS:        printf("MINUS"); break;
        case TOKEN_STAR:         printf("STAR"); break;
        case TOKEN_DIV:          printf("DIV"); break;
        case TOKEN_ASSIGNOP:     printf("ASSIGNOP"); break;
        case TOKEN_RELOP:        printf("RELOP"); break;
        case TOKEN_AND:          printf("AND"); break;
        case TOKEN_OR:           printf("OR"); break;
        case TOKEN_NOT:          printf("NOT"); break;
        case TOKEN_LP:           printf("LP"); break;
        case TOKEN_RP:           printf("RP"); break;
        case TOKEN_LC:           printf("LC"); break;
        case TOKEN_RC:           printf("RC"); break;
        case TOKEN_LB:           printf("LB"); break;
        case TOKEN_RB:           printf("RB"); break;
        case TOKEN_COMMA:        printf("COMMA"); break;
        case TOKEN_SEMI:         printf("SEMI"); break;
        case TOKEN_DOT:          printf("DOT"); break;
        case TOKEN_STRUCT:       printf("STRUCT"); break;
        case TOKEN_RETURN:       printf("RETURN"); break;
        case TOKEN_IF:           printf("IF"); break;
        case TOKEN_ELSE:         printf("ELSE"); break;
        case TOKEN_WHILE:        printf("WHILE"); break;
        case TOKEN_EOF:          printf("EOF"); break;
        default:                 printf("UNKNOWN"); break;
    }
    
    if (token.value[0] != '\0') {
        printf(",\"%s\"", token.value);
    } else if (token.type == TOKEN_INT) {
        printf(",%d", token.intVal);
    } else if (token.type == TOKEN_FLOAT) {
        printf(",%f", token.floatVal);
    }
    
    printf(")\n");
}

int lex_wrap() {
    return 1;
}
