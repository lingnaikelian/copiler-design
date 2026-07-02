%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "../scanner/common.h"

extern int errorCount;
extern int lineNumber;

ASTNode* programRoot = NULL;

/* yyerror 只增加错误计数，不输出信息 */
void yyerror(const char* msg) {
    errorCount++;
}
%}

%locations

%union {
    int intVal;
    float floatVal;
    char* strVal;
    ASTNode* node;
}

%token <strVal> ID TYPE
%token <intVal> INT
%token <floatVal> FLOAT
%token <strVal> RELOP
%token SEMI COMMA ASSIGNOP PLUS MINUS STAR DIV AND OR DOT NOT
%token LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE

%type <node> Program ExtDefList ExtDef ExtDecList
%type <node> Specifier StructSpecifier OptTag Tag
%type <node> VarDec FunDec VarList ParamDec
%type <node> CompSt StmtList Stmt
%type <node> DefList Def DecList Dec
%type <node> Exp Args

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT UMINUS
%left LP RP LB RB DOT

%%

Program : ExtDefList { $$ = createNode(NODE_PROGRAM, $1 ? $1->line : 1); addChild($$, $1); programRoot = $$; }
        ;

ExtDefList : ExtDef ExtDefList { $$ = createNode(NODE_EXTDEFLIST, $1->line); addChild($$, $1); if ($2) addChild($$, $2); }
           | { $$ = NULL; }
           ;

ExtDef : Specifier ExtDecList SEMI { $$ = createNode(NODE_EXTDEF, $1->line); addChild($$, $1); addChild($$, $2); addChild($$, createToken($1->line, SEMI)); }
       | Specifier SEMI { $$ = createNode(NODE_EXTDEF, $1->line); addChild($$, $1); addChild($$, createToken($1->line, SEMI)); }
       | Specifier FunDec CompSt { $$ = createNode(NODE_EXTDEF, $1->line); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
       | Specifier FunDec SEMI { $$ = createNode(NODE_EXTDEF, $1->line); addChild($$, $1); addChild($$, $2); addChild($$, createToken($1->line, SEMI)); }
       ;

Specifier : TYPE { $$ = createNode(NODE_SPECIFIER, @1.first_line); addChild($$, createTokenStr(@1.first_line, TYPE, $1)); }
          | StructSpecifier { $$ = $1; }
          ;

StructSpecifier : STRUCT OptTag LC DefList RC { $$ = createNode(NODE_STRUCTSPECIFIER, @1.first_line); addChild($$, createToken(@1.first_line, STRUCT)); addChild($$, $2); addChild($$, createToken(@3.first_line, LC)); addChild($$, $4); addChild($$, createToken(@5.first_line, RC)); }
                | STRUCT Tag { $$ = createNode(NODE_STRUCTSPECIFIER, @1.first_line); addChild($$, createToken(@1.first_line, STRUCT)); addChild($$, $2); }
                ;

OptTag : ID { $$ = createNode(NODE_OPTTAG, @1.first_line); addChild($$, createTokenStr(@1.first_line, ID, $1)); }
       | { $$ = NULL; }
       ;

Tag : ID { $$ = createNode(NODE_TAG, @1.first_line); addChild($$, createTokenStr(@1.first_line, ID, $1)); }
    ;

ExtDecList : VarDec { $$ = createNode(NODE_EXTDECLIST, $1->line); addChild($$, $1); }
           | VarDec COMMA ExtDecList { $$ = createNode(NODE_EXTDECLIST, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, COMMA)); addChild($$, $3); }
           ;

VarDec : ID { $$ = createNode(NODE_VARDEC, @1.first_line); addChild($$, createTokenStr(@1.first_line, ID, $1)); }
       | VarDec LB INT RB { $$ = createNode(NODE_VARDEC, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, LB)); addChild($$, createTokenInt(@3.first_line, INT, $3)); addChild($$, createToken(@4.first_line, RB)); }
       ;

FunDec : ID LP VarList RP { $$ = createNode(NODE_FUNDEC, @1.first_line); addChild($$, createTokenStr(@1.first_line, ID, $1)); addChild($$, createToken(@2.first_line, LP)); addChild($$, $3); addChild($$, createToken(@4.first_line, RP)); }
       | ID LP RP { $$ = createNode(NODE_FUNDEC, @1.first_line); addChild($$, createTokenStr(@1.first_line, ID, $1)); addChild($$, createToken(@2.first_line, LP)); addChild($$, createToken(@3.first_line, RP)); }
       ;

VarList : ParamDec COMMA VarList { $$ = createNode(NODE_VARLIST, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, COMMA)); addChild($$, $3); }
        | ParamDec { $$ = createNode(NODE_VARLIST, $1->line); addChild($$, $1); }
        ;

ParamDec : Specifier VarDec { $$ = createNode(NODE_PARAMDEC, $1->line); addChild($$, $1); addChild($$, $2); }
         ;

CompSt : LC DefList StmtList RC { $$ = createNode(NODE_COMPST, @1.first_line); addChild($$, createToken(@1.first_line, LC)); addChild($$, $2); addChild($$, $3); addChild($$, createToken(@4.first_line, RC)); }
       ;

StmtList : Stmt StmtList { $$ = createNode(NODE_STMTLIST, $1->line); addChild($$, $1); if ($2) addChild($$, $2); }
         | { $$ = NULL; }
         ;

/* ----- Stmt 规则（含精确错误恢复和通用恢复）----- */
Stmt : Exp SEMI { $$ = createNode(NODE_STMT, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, SEMI)); }
     | CompSt { $$ = $1; }
     | RETURN Exp SEMI { $$ = createNode(NODE_STMT, @1.first_line); addChild($$, createToken(@1.first_line, RETURN)); addChild($$, $2); addChild($$, createToken(@3.first_line, SEMI)); }
     | IF LP Exp RP Stmt { $$ = createNode(NODE_STMT, @1.first_line); addChild($$, createToken(@1.first_line, IF)); addChild($$, createToken(@2.first_line, LP)); addChild($$, $3); addChild($$, createToken(@4.first_line, RP)); addChild($$, $5); }
     | IF LP Exp RP Stmt ELSE Stmt { $$ = createNode(NODE_STMT, @1.first_line); addChild($$, createToken(@1.first_line, IF)); addChild($$, createToken(@2.first_line, LP)); addChild($$, $3); addChild($$, createToken(@4.first_line, RP)); addChild($$, $5); addChild($$, createToken(@6.first_line, ELSE)); addChild($$, $7); }
     /* 精确捕获 if 语句中缺少分号 */
     | IF LP Exp RP Exp error ELSE Stmt {
          fprintf(stderr, "Error type B at Line %d: Missing \";\".\n", lineNumber);
          errorCount++;
          yyclearin;
          yyerrok;
          $$ = createNode(NODE_STMT, lineNumber);
        }
     | WHILE LP Exp RP Stmt { $$ = createNode(NODE_STMT, @1.first_line); addChild($$, createToken(@1.first_line, WHILE)); addChild($$, createToken(@2.first_line, LP)); addChild($$, $3); addChild($$, createToken(@4.first_line, RP)); addChild($$, $5); }
     /* 通用错误恢复：遇到无法解析的语句，跳过到分号或右花括号，不输出错误（因为 yyerror 已不输出） */
     | error SEMI { $$ = createNode(NODE_STMT, lineNumber); }  /* 不输出，仅恢复 */
     | error RC { $$ = createNode(NODE_STMT, lineNumber); }    /* 不输出，仅恢复 */
     ;

DefList : Def DefList { $$ = createNode(NODE_DEFLIST, $1->line); addChild($$, $1); if ($2) addChild($$, $2); }
        | { $$ = NULL; }
        ;

Def : Specifier DecList SEMI { $$ = createNode(NODE_DEF, $1->line); addChild($$, $1); addChild($$, $2); addChild($$, createToken(@3.first_line, SEMI)); }
    ;

DecList : Dec { $$ = createNode(NODE_DECLIST, $1->line); addChild($$, $1); }
        | Dec COMMA DecList { $$ = createNode(NODE_DECLIST, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, COMMA)); addChild($$, $3); }
        ;

Dec : VarDec { $$ = createNode(NODE_DEC, $1->line); addChild($$, $1); }
    | VarDec ASSIGNOP Exp { $$ = createNode(NODE_DEC, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, ASSIGNOP)); addChild($$, $3); }
    ;

/* ----- Exp 规则（含精确错误恢复）----- */
Exp : Exp ASSIGNOP Exp { $$ = createNode(NODE_EXP, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, ASSIGNOP)); addChild($$, $3); }
    | Exp AND Exp { $$ = createNode(NODE_EXP, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, AND)); addChild($$, $3); }
    | Exp OR Exp { $$ = createNode(NODE_EXP, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, OR)); addChild($$, $3); }
    | Exp RELOP Exp { $$ = createNode(NODE_EXP, $1->line); addChild($$, $1); addChild($$, createTokenStr(@2.first_line, RELOP, $2)); addChild($$, $3); }
    | Exp PLUS Exp { $$ = createNode(NODE_EXP, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, PLUS)); addChild($$, $3); }
    | Exp MINUS Exp { $$ = createNode(NODE_EXP, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, MINUS)); addChild($$, $3); }
    | Exp STAR Exp { $$ = createNode(NODE_EXP, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, STAR)); addChild($$, $3); }
    | Exp DIV Exp { $$ = createNode(NODE_EXP, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, DIV)); addChild($$, $3); }
    | LP Exp RP { $$ = createNode(NODE_EXP, @1.first_line); addChild($$, createToken(@1.first_line, LP)); addChild($$, $2); addChild($$, createToken(@3.first_line, RP)); }
    | MINUS Exp %prec UMINUS { $$ = createNode(NODE_EXP, @1.first_line); addChild($$, createToken(@1.first_line, MINUS)); addChild($$, $2); }
    | NOT Exp { $$ = createNode(NODE_EXP, @1.first_line); addChild($$, createToken(@1.first_line, NOT)); addChild($$, $2); }
    | ID LP Args RP { $$ = createNode(NODE_EXP, @1.first_line); addChild($$, createTokenStr(@1.first_line, ID, $1)); addChild($$, createToken(@2.first_line, LP)); addChild($$, $3); addChild($$, createToken(@4.first_line, RP)); }
    | ID LP RP { $$ = createNode(NODE_EXP, @1.first_line); addChild($$, createTokenStr(@1.first_line, ID, $1)); addChild($$, createToken(@2.first_line, LP)); addChild($$, createToken(@3.first_line, RP)); }
    /* 正确的数组访问 */
    | Exp LB Exp RB { $$ = createNode(NODE_EXP, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, LB)); addChild($$, $3); addChild($$, createToken(@4.first_line, RB)); }
    /* 精确捕获数组访问缺少 ] */
    | Exp LB Exp error {
          fprintf(stderr, "Error type B at Line %d: Missing \"]\".\n", lineNumber);
          errorCount++;
          yyclearin;
          yyerrok;
          $$ = createNode(NODE_EXP, lineNumber);
          addChild($$, $1);
          addChild($$, createToken(@2.first_line, LB));
          addChild($$, $3);
        }
    | Exp DOT ID { $$ = createNode(NODE_EXP, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, DOT)); addChild($$, createTokenStr(@3.first_line, ID, $3)); }
    | ID { $$ = createNode(NODE_EXP, @1.first_line); addChild($$, createTokenStr(@1.first_line, ID, $1)); }
    | INT { $$ = createNode(NODE_EXP, @1.first_line); addChild($$, createTokenInt(@1.first_line, INT, $1)); }
    | FLOAT { $$ = createNode(NODE_EXP, @1.first_line); addChild($$, createTokenFloat(@1.first_line, FLOAT, $1)); }
    ;

Args : Exp COMMA Args { $$ = createNode(NODE_ARGS, $1->line); addChild($$, $1); addChild($$, createToken(@2.first_line, COMMA)); addChild($$, $3); }
     | Exp { $$ = createNode(NODE_ARGS, $1->line); addChild($$, $1); }
     ;

%%
