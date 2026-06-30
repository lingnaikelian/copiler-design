#include "node.h"
#include "parser.tab.h"   // 必须包含，以获得 ID, INT, FLOAT 等宏定义
#include <string.h>
#include <stdlib.h>

ASTNode* createNode(NodeType type, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->nodeType = type;
    node->line = line;
    node->tokenType = 0;
    node->childCount = 0;
    node->children = NULL;
    node->attr.value = NULL;
    node->intValue = 0;
    node->floatValue = 0.0f;
    return node;
}

void addChild(ASTNode* parent, ASTNode* child) {
    if (child == NULL) return;
    parent->children = (ASTNode**)realloc(parent->children,
                                          (parent->childCount + 1) * sizeof(ASTNode*));
    parent->children[parent->childCount++] = child;
}

void freeAST(ASTNode* root) {
    if (root == NULL) return;
    for (int i = 0; i < root->childCount; i++) {
        freeAST(root->children[i]);
    }
    if (root->attr.value != NULL) {
        free(root->attr.value);
    }
    free(root->children);
    free(root);
}

const char* getNodeTypeName(NodeType type) {
    switch (type) {
        case NODE_PROGRAM:         return "Program";
        case NODE_EXTDEFLIST:      return "ExtDefList";
        case NODE_EXTDEF:          return "ExtDef";
        case NODE_EXTDECLIST:      return "ExtDecList";
        case NODE_SPECIFIER:       return "Specifier";
        case NODE_STRUCTSPECIFIER: return "StructSpecifier";
        case NODE_OPTTAG:          return "OptTag";
        case NODE_TAG:             return "Tag";
        case NODE_VARDEC:          return "VarDec";
        case NODE_FUNDEC:          return "FunDec";
        case NODE_VARLIST:         return "VarList";
        case NODE_PARAMDEC:        return "ParamDec";
        case NODE_COMPST:          return "CompSt";
        case NODE_STMTLIST:        return "StmtList";
        case NODE_STMT:            return "Stmt";
        case NODE_DEFLIST:         return "DefList";
        case NODE_DEF:             return "Def";
        case NODE_DECLIST:         return "DecList";
        case NODE_DEC:             return "Dec";
        case NODE_EXP:             return "Exp";
        case NODE_ARGS:            return "Args";
        case NODE_TOKEN:           return "TOKEN";
        default:                   return "UNKNOWN";
    }
}

const char* getTokenName(int tokenType) {
    switch (tokenType) {
        case ID:     return "ID";
        case TYPE:   return "TYPE";
        case INT:    return "INT";
        case FLOAT:  return "FLOAT";
        case SEMI:   return "SEMI";
        case COMMA:  return "COMMA";
        case ASSIGNOP: return "ASSIGNOP";
        case RELOP:  return "RELOP";
        case PLUS:   return "PLUS";
        case MINUS:  return "MINUS";
        case STAR:   return "STAR";
        case DIV:    return "DIV";
        case AND:    return "AND";
        case OR:     return "OR";
        case DOT:    return "DOT";
        case NOT:    return "NOT";
        case LP:     return "LP";
        case RP:     return "RP";
        case LB:     return "LB";
        case RB:     return "RB";
        case LC:     return "LC";
        case RC:     return "RC";
        case STRUCT: return "STRUCT";
        case RETURN: return "RETURN";
        case IF:     return "IF";
        case ELSE:   return "ELSE";
        case WHILE:  return "WHILE";
        default: {
            static char buf[32];
            snprintf(buf, sizeof(buf), "TOKEN(%d)", tokenType);
            return buf;
        }
    }
}

void printAST(ASTNode* root, int indent) {
    if (root == NULL) return;

    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    if (root->nodeType == NODE_TOKEN) {
        int tokenType = root->tokenType;
        const char* name = getTokenName(tokenType);
        printf("%s", name);

        if (tokenType == ID || tokenType == TYPE) {
            if (root->attr.value != NULL)
                printf(": %s", root->attr.value);
        } else if (tokenType == INT) {
            printf(": %d", root->intValue);
        } else if (tokenType == FLOAT) {
            printf(": %f", root->floatValue);
        }
        printf("\n");
        return;
    } else {
        printf("%s (%d)\n", getNodeTypeName(root->nodeType), root->line);
        for (int i = 0; i < root->childCount; i++) {
            if (root->children[i] != NULL)
                printAST(root->children[i], indent + 1);
        }
    }
}

/* -------- Token creation helpers (implemented here) -------- */
ASTNode* createToken(int line, int tokenType) {
    ASTNode* tok = createNode(NODE_TOKEN, line);
    tok->tokenType = tokenType;
    tok->attr.value = NULL;
    return tok;
}

ASTNode* createTokenStr(int line, int tokenType, char* str) {
    ASTNode* tok = createNode(NODE_TOKEN, line);
    tok->tokenType = tokenType;
    tok->attr.value = str ? strdup(str) : NULL;
    return tok;
}

ASTNode* createTokenInt(int line, int tokenType, int val) {
    ASTNode* tok = createNode(NODE_TOKEN, line);
    tok->tokenType = tokenType;
    tok->intValue = val;
    return tok;
}

ASTNode* createTokenFloat(int line, int tokenType, float val) {
    ASTNode* tok = createNode(NODE_TOKEN, line);
    tok->tokenType = tokenType;
    tok->floatValue = val;
    return tok;
}
