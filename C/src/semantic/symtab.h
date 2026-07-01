#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_FUNCTION
} BaseType;

typedef struct Type {
    BaseType baseType;
    int arraySize;
    struct Type* elementType;
    char* structName;
    struct SymbolTable* fieldTable;
} Type;

typedef enum {
    KIND_VARIABLE,
    KIND_FUNCTION,
    KIND_STRUCT,
    KIND_PARAMETER
} SymbolKind;

typedef struct Symbol {
    char* name;
    SymbolKind kind;
    Type* type;
    int line;
    struct Symbol* next;
    int paramCount;
    Type** paramTypes;
} Symbol;

typedef struct SymbolTable {
    Symbol* head;
    struct SymbolTable* parent;
} SymbolTable;

extern SymbolTable* currentTable;

Type* createIntType();
Type* createFloatType();
Type* createArrayType(Type* elementType, int size);
Type* createStructType(char* name);
Type* createFunctionType(Type* returnType, int paramCount, Type** paramTypes);
void freeType(Type* type);
int compareTypes(Type* t1, Type* t2);
const char* typeToString(Type* type);

SymbolTable* createSymbolTable(SymbolTable* parent);
void insertSymbol(SymbolTable* table, Symbol* symbol);
Symbol* lookupSymbol(SymbolTable* table, const char* name);
void deleteSymbolTable(SymbolTable* table);
Symbol* createSymbol(char* name, SymbolKind kind, Type* type, int line);

#endif