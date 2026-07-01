#include "symtab.h"

SymbolTable* currentTable = NULL;

Type* createIntType() {
    Type* t = (Type*)malloc(sizeof(Type));
    t->baseType = TYPE_INT;
    t->arraySize = 0;
    t->elementType = NULL;
    t->structName = NULL;
    return t;
}

Type* createFloatType() {
    Type* t = (Type*)malloc(sizeof(Type));
    t->baseType = TYPE_FLOAT;
    t->arraySize = 0;
    t->elementType = NULL;
    t->structName = NULL;
    return t;
}

Type* createArrayType(Type* elementType, int size) {
    Type* t = (Type*)malloc(sizeof(Type));
    t->baseType = TYPE_ARRAY;
    t->arraySize = size;
    t->elementType = elementType;
    t->structName = NULL;
    return t;
}

Type* createStructType(char* name) {
    Type* t = (Type*)malloc(sizeof(Type));
    t->baseType = TYPE_STRUCT;
    t->arraySize = 0;
    t->elementType = NULL;
    t->structName = strdup(name);
    t->fieldTable = NULL;
    return t;
}

Type* createFunctionType(Type* returnType, int paramCount, Type** paramTypes) {
    Type* t = (Type*)malloc(sizeof(Type));
    t->baseType = TYPE_FUNCTION;
    t->arraySize = paramCount;
    t->elementType = returnType;
    t->structName = NULL;
    return t;
}

void freeType(Type* type) {
    if (type == NULL) return;
    if (type->structName) free(type->structName);
    if (type->elementType) freeType(type->elementType);
    free(type);
}

int compareTypes(Type* t1, Type* t2) {
    if (t1 == NULL || t2 == NULL) return 0;
    if (t1->baseType != t2->baseType) return 0;
    switch (t1->baseType) {
        case TYPE_INT:
        case TYPE_FLOAT:
            return 1;
        case TYPE_ARRAY:
            return t1->arraySize == t2->arraySize && compareTypes(t1->elementType, t2->elementType);
        case TYPE_STRUCT:
            return t1->structName && t2->structName && strcmp(t1->structName, t2->structName) == 0;
        case TYPE_FUNCTION:
            return t1->arraySize == t2->arraySize && compareTypes(t1->elementType, t2->elementType);
        default:
            return 0;
    }
}

const char* typeToString(Type* type) {
    static char buf[256];
    if (type == NULL) return "unknown";
    switch (type->baseType) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_ARRAY:
            snprintf(buf, 256, "%s[%d]", typeToString(type->elementType), type->arraySize);
            return buf;
        case TYPE_STRUCT:
            snprintf(buf, 256, "struct %s", type->structName);
            return buf;
        case TYPE_FUNCTION:
            return "function";
        default: return "unknown";
    }
}

SymbolTable* createSymbolTable(SymbolTable* parent) {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    table->head = NULL;
    table->parent = parent;
    return table;
}

void insertSymbol(SymbolTable* table, Symbol* symbol) {
    symbol->next = table->head;
    table->head = symbol;
}

Symbol* lookupSymbol(SymbolTable* table, const char* name) {
    while (table != NULL) {
        Symbol* s = table->head;
        while (s != NULL) {
            if (strcmp(s->name, name) == 0) return s;
            s = s->next;
        }
        table = table->parent;
    }
    return NULL;
}

void deleteSymbolTable(SymbolTable* table) {
    Symbol* s = table->head;
    while (s != NULL) {
        Symbol* next = s->next;
        free(s->name);
        freeType(s->type);
        free(s);
        s = next;
    }
    free(table);
}

Symbol* createSymbol(char* name, SymbolKind kind, Type* type, int line) {
    Symbol* s = (Symbol*)malloc(sizeof(Symbol));
    s->name = strdup(name);
    s->kind = kind;
    s->type = type;
    s->line = line;
    s->next = NULL;
    s->paramCount = 0;
    s->paramTypes = (Type**)malloc(sizeof(Type*) * 32);
    return s;
}