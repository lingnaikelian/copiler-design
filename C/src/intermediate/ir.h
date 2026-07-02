#ifndef IR_H
#define IR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/node.h"
#include "../parser/parser.tab.h"
#include "../semantic/symtab.h"

typedef enum {
    IR_LABEL,
    IR_FUNCTION,
    IR_ASSIGN,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_ADDR,
    IR_DEREF,
    IR_DEREF_ASSIGN,
    IR_GOTO,
    IR_IF,
    IR_RETURN,
    IR_DEC,
    IR_ARG,
    IR_CALL,
    IR_PARAM,
    IR_READ,
    IR_WRITE
} IRType;

typedef enum {
    RELOP_EQ,
    RELOP_NE,
    RELOP_LT,
    RELOP_GT,
    RELOP_LE,
    RELOP_GE
} RelOp;

typedef struct IRNode {
    IRType type;
    char* result;
    char* arg1;
    char* arg2;
    char* arg3;
    RelOp relop;
    struct IRNode* next;
} IRNode;

extern IRNode* irList;
extern int tempCount;
extern int labelCount;

IRNode* createIRNode(IRType type);
void addIRNode(IRNode* node);
void printIR(FILE* fp);
void freeIR();

char* newTemp();
char* newLabel();

void genIR(ASTNode* root);
void genExtDefList(ASTNode* node);
void genExtDef(ASTNode* node);
char* genExp(ASTNode* node);
void genStmt(ASTNode* node);
void genCompSt(ASTNode* node);
void genStmtList(ASTNode* node);
void genDefList(ASTNode* node);
void genDef(ASTNode* node);
void genDecList(ASTNode* node);
void genDec(ASTNode* node);

#endif
