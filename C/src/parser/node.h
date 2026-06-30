#ifndef NODE_H
#define NODE_H

#include <stdio.h>

typedef enum {
    NODE_PROGRAM,
    NODE_EXTDEFLIST,
    NODE_EXTDEF,
    NODE_EXTDECLIST,
    NODE_SPECIFIER,
    NODE_STRUCTSPECIFIER,
    NODE_OPTTAG,
    NODE_TAG,
    NODE_VARDEC,
    NODE_FUNDEC,
    NODE_VARLIST,
    NODE_PARAMDEC,
    NODE_COMPST,
    NODE_STMTLIST,
    NODE_STMT,
    NODE_DEFLIST,
    NODE_DEF,
    NODE_DECLIST,
    NODE_DEC,
    NODE_EXP,
    NODE_ARGS,
    NODE_TOKEN
} NodeType;

/* Attr only stores string value (for ID/TYPE) */
typedef union {
    char* value;
} Attr;

typedef struct ASTNode {
    NodeType nodeType;
    int line;
    int tokenType;          // used only for NODE_TOKEN: stores token type (ID, INT, etc.)
    struct ASTNode** children;
    int childCount;
    Attr attr;              // string value for ID/TYPE
    int intValue;           // integer constant
    float floatValue;       // float constant
} ASTNode;

extern int errorCount;

ASTNode* createNode(NodeType type, int line);
void addChild(ASTNode* parent, ASTNode* child);
void freeAST(ASTNode* root);
const char* getNodeTypeName(NodeType type);
const char* getTokenName(int tokenType);
void printAST(ASTNode* root, int indent);

/* Helper functions to create token nodes */
ASTNode* createToken(int line, int tokenType);
ASTNode* createTokenStr(int line, int tokenType, char* str);
ASTNode* createTokenInt(int line, int tokenType, int val);
ASTNode* createTokenFloat(int line, int tokenType, float val);

#endif
