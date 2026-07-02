#include "semantic.h"

extern int errorCount;

static Type* currentReturnType = NULL;
static int inFunction = 0;

static Type* getTypeFromSpecifier(ASTNode* specifier);
static Type* getTypeFromVarDec(ASTNode* varDec, Type* baseType);
static Type* getExpressionType(ASTNode* exp);
static void traverseExp(ASTNode* exp);
static void traverseStmt(ASTNode* stmt);
static void traverseDef(ASTNode* def);
static void traverseExtDef(ASTNode* extDef);
static void processStructFields(ASTNode* defList, Symbol* structSym, int structLine);

static Type* getTypeFromSpecifier(ASTNode* specifier) {
    if (specifier == NULL) return NULL;
    if (specifier->childCount < 1) return NULL;
    ASTNode* child = specifier->children[0];
    if (child->tokenType == TYPE) {
        if (strcmp(child->attr.value, "int") == 0) return createIntType();
        if (strcmp(child->attr.value, "float") == 0) return createFloatType();
    } else if (child->tokenType == STRUCT) {
        ASTNode* tag = NULL;
        if (specifier->childCount > 1) {
            tag = specifier->children[1];
        }
        if (tag != NULL && tag->childCount > 0) {
            char* name = tag->children[0]->attr.value;
            Symbol* sym = lookupSymbol(currentTable, name);
            if (sym != NULL && sym->kind == KIND_STRUCT) {
                return sym->type;
            } else {
                fprintf(stderr, "Error type 17 at Line %d: Undefined structure \"%s\".\n", specifier->line, name);
                errorCount++;
                return createStructType(name);
            }
        }
    }
    return NULL;
}

static Type* getTypeFromVarDec(ASTNode* varDec, Type* baseType) {
    if (varDec == NULL) return NULL;
    if (varDec->nodeType == NODE_VARDEC) {
        if (varDec->childCount == 1) {
            return baseType;
        } else {
            ASTNode* intNode = varDec->children[2];
            int size = 0;
            if (intNode != NULL) {
                size = intNode->intValue;
            }
            Type* elementType = getTypeFromVarDec(varDec->children[0], baseType);
            return createArrayType(elementType, size);
        }
    }
    return baseType;
}

static char* getVarDecName(ASTNode* varDec) {
    if (varDec == NULL) return NULL;
    if (varDec->nodeType == NODE_VARDEC) {
        if (varDec->childCount >= 1) {
            ASTNode* idNode = varDec->children[0];
            if (idNode != NULL && idNode->tokenType == ID) {
                return idNode->attr.value;
            }
            return getVarDecName(varDec->children[0]);
        }
    }
    if (varDec->nodeType == NODE_TOKEN && varDec->tokenType == ID) {
        return varDec->attr.value;
    }
    return NULL;
}

static Type* getExpressionType(ASTNode* exp) {
    if (exp == NULL) return NULL;
    if (exp->nodeType == NODE_TOKEN) {
        switch (exp->tokenType) {
            case INT: return createIntType();
            case FLOAT: return createFloatType();
            case ID: {
                Symbol* sym = lookupSymbol(currentTable, exp->attr.value);
                if (sym != NULL && (sym->kind == KIND_VARIABLE || sym->kind == KIND_PARAMETER)) {
                    return sym->type;
                }
                return NULL;
            }
            default: return NULL;
        }
    }
    if (exp->nodeType != NODE_EXP) return NULL;
    ASTNode* op = NULL;
    if (exp->childCount > 1) {
        op = exp->children[1];
    }
    if (op != NULL) {
        switch (op->tokenType) {
            case ASSIGNOP: {
                if (exp->childCount >= 1) {
                    Type* leftType = getExpressionType(exp->children[0]);
                    return leftType;
                }
                return NULL;
            }
            case PLUS: case STAR: case DIV: {
                if (exp->childCount >= 3) {
                    Type* leftType = getExpressionType(exp->children[0]);
                    Type* rightType = getExpressionType(exp->children[2]);
                    if (leftType != NULL && rightType != NULL) {
                        if (leftType->baseType == TYPE_FLOAT || rightType->baseType == TYPE_FLOAT) {
                            return createFloatType();
                        }
                        return createIntType();
                    }
                }
                return NULL;
            }
            case MINUS: {
                if (exp->childCount >= 3) {
                    Type* leftType = getExpressionType(exp->children[0]);
                    Type* rightType = getExpressionType(exp->children[2]);
                    if (leftType != NULL && rightType != NULL) {
                        if (leftType->baseType == TYPE_FLOAT || rightType->baseType == TYPE_FLOAT) {
                            return createFloatType();
                        }
                        return createIntType();
                    }
                } else if (exp->childCount >= 2) {
                    Type* rightType = getExpressionType(exp->children[1]);
                    if (rightType != NULL) {
                        return rightType->baseType == TYPE_FLOAT ? createFloatType() : createIntType();
                    }
                }
                return NULL;
            }
            case RELOP: case AND: case OR: {
                return createIntType();
            }
            case NOT: {
                return createIntType();
            }
            case DOT: {
                if (exp->childCount >= 3) {
                    Type* leftType = getExpressionType(exp->children[0]);
                    if (leftType != NULL && leftType->baseType == TYPE_STRUCT) {
                        ASTNode* fieldNode = exp->children[2];
                        if (fieldNode != NULL && fieldNode->tokenType == ID) {
                            Symbol* structSym = lookupSymbol(currentTable, leftType->structName);
                            if (structSym != NULL && structSym->kind == KIND_STRUCT && structSym->type != NULL && structSym->type->fieldTable != NULL) {
                                Symbol* field = lookupSymbol(structSym->type->fieldTable, fieldNode->attr.value);
                                if (field != NULL) {
                                    return field->type;
                                }
                            }
                        }
                    }
                }
                return NULL;
            }
            case LB: {
                if (exp->childCount >= 1) {
                    Type* leftType = getExpressionType(exp->children[0]);
                    if (leftType != NULL && leftType->baseType == TYPE_ARRAY) {
                        return leftType->elementType;
                    }
                }
                return NULL;
            }
            case LP: {
                if (exp->childCount >= 1) {
                    ASTNode* idNode = exp->children[0];
                    if (idNode != NULL && idNode->tokenType == ID) {
                        Symbol* sym = lookupSymbol(currentTable, idNode->attr.value);
                        if (sym != NULL && sym->kind == KIND_FUNCTION) {
                            return sym->type->elementType;
                        }
                    }
                }
                return NULL;
            }
            default: break;
        }
    }
    if (exp->childCount >= 1) {
        return getExpressionType(exp->children[0]);
    }
    return NULL;
}

static int isLValue(ASTNode* exp) {
    if (exp == NULL) return 0;
    if (exp->nodeType == NODE_TOKEN && exp->tokenType == ID) {
        return 1;
    }
    if (exp->nodeType == NODE_EXP) {
        ASTNode* op = NULL;
        if (exp->childCount > 1) {
            op = exp->children[1];
        }
        if (op != NULL) {
            if (op->tokenType == DOT) return 1;
            if (op->tokenType == LB) return 1;
        }
        if (exp->childCount >= 1) {
            return isLValue(exp->children[0]);
        }
    }
    return 0;
}

static void processStructFieldsRecursive(ASTNode* defList, SymbolTable* fieldTable) {
    if (defList == NULL) return;
    if (defList->nodeType != NODE_DEFLIST) return;
    
    ASTNode* def = defList->children[0];
    if (def != NULL && def->nodeType == NODE_DEF) {
        ASTNode* specifier = def->children[0];
        ASTNode* decList = def->children[1];
        Type* baseType = getTypeFromSpecifier(specifier);
        if (decList != NULL && decList->nodeType == NODE_DECLIST) {
            for (int i = 0; i < decList->childCount; i += 2) {
                ASTNode* dec = decList->children[i];
                if (dec != NULL && dec->nodeType == NODE_DEC) {
                    ASTNode* varDec = dec->children[0];
                    char* name = getVarDecName(varDec);
                    if (name != NULL) {
                        Symbol* existing = lookupSymbol(fieldTable, name);
                        if (existing != NULL) {
                            fprintf(stderr, "Error type 15 at Line %d: Redefined field \"%s\".\n", def->line, name);
                            errorCount++;
                        } else {
                            Type* fieldType = getTypeFromVarDec(varDec, baseType);
                            Symbol* fieldSym = createSymbol(name, KIND_VARIABLE, fieldType, def->line);
                            insertSymbol(fieldTable, fieldSym);
                        }
                    }
                }
            }
        }
    }
    
    ASTNode* nextDefList = defList->children[1];
    if (nextDefList != NULL) {
        processStructFieldsRecursive(nextDefList, fieldTable);
    }
}

static void processStructFields(ASTNode* defList, Symbol* structSym, int structLine) {
    if (defList == NULL || structSym == NULL) return;
    SymbolTable* fieldTable = createSymbolTable(NULL);
    structSym->type->fieldTable = fieldTable;
    processStructFieldsRecursive(defList, fieldTable);
}

static void traverseExp(ASTNode* exp) {
    if (exp == NULL) return;
    if (exp->nodeType == NODE_TOKEN) {
        if (exp->tokenType == ID) {
            char* name = exp->attr.value;
            if (strcmp(name, "read") == 0 || strcmp(name, "write") == 0) {
                return;
            }
            Symbol* sym = lookupSymbol(currentTable, name);
            if (sym == NULL) {
                fprintf(stderr, "Error type 1 at Line %d: Undefined variable \"%s\".\n", exp->line, name);
                errorCount++;
            }
        }
        return;
    }
    if (exp->nodeType != NODE_EXP) return;
    
    ASTNode* op = NULL;
    int isFuncCall = 0;
    int isDot = 0;
    
    if (exp->childCount > 1) {
        op = exp->children[1];
        isFuncCall = (op != NULL && op->tokenType == LP);
        isDot = (op != NULL && op->tokenType == DOT);
    }
    
    for (int i = 0; i < exp->childCount; i++) {
        if (isFuncCall && i == 0) continue;
        if (isDot && i == 2) continue;
        traverseExp(exp->children[i]);
    }
    
    if (op != NULL) {
        switch (op->tokenType) {
            case ASSIGNOP: {
                if (exp->childCount >= 3) {
                    if (!isLValue(exp->children[0])) {
                        fprintf(stderr, "Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", exp->line);
                        errorCount++;
                    }
                    Type* leftType = getExpressionType(exp->children[0]);
                    Type* rightType = getExpressionType(exp->children[2]);
                    if (leftType != NULL && rightType != NULL && !compareTypes(leftType, rightType)) {
                        fprintf(stderr, "Error type 5 at Line %d: Type mismatched for assignment.\n", exp->line);
                        errorCount++;
                    }
                }
                break;
            }
            case PLUS: case STAR: case DIV: {
                if (exp->childCount >= 3) {
                    Type* leftType = getExpressionType(exp->children[0]);
                    Type* rightType = getExpressionType(exp->children[2]);
                    if (leftType != NULL && rightType != NULL) {
                        if ((leftType->baseType == TYPE_INT && rightType->baseType == TYPE_FLOAT) ||
                            (leftType->baseType == TYPE_FLOAT && rightType->baseType == TYPE_INT)) {
                            fprintf(stderr, "Error type 7 at Line %d: Type mismatched for operands.\n", exp->line);
                            errorCount++;
                        }
                        if (leftType->baseType != TYPE_INT && leftType->baseType != TYPE_FLOAT) {
                            fprintf(stderr, "Error type 7 at Line %d: Type mismatched for operands.\n", exp->line);
                            errorCount++;
                        }
                        if (rightType->baseType != TYPE_INT && rightType->baseType != TYPE_FLOAT) {
                            fprintf(stderr, "Error type 7 at Line %d: Type mismatched for operands.\n", exp->line);
                            errorCount++;
                        }
                    }
                }
                break;
            }
            case MINUS: {
                if (exp->childCount >= 3) {
                    Type* leftType = getExpressionType(exp->children[0]);
                    Type* rightType = getExpressionType(exp->children[2]);
                    if (leftType != NULL && rightType != NULL) {
                        if ((leftType->baseType == TYPE_INT && rightType->baseType == TYPE_FLOAT) ||
                            (leftType->baseType == TYPE_FLOAT && rightType->baseType == TYPE_INT)) {
                            fprintf(stderr, "Error type 7 at Line %d: Type mismatched for operands.\n", exp->line);
                            errorCount++;
                        }
                        if (leftType->baseType != TYPE_INT && leftType->baseType != TYPE_FLOAT) {
                            fprintf(stderr, "Error type 7 at Line %d: Type mismatched for operands.\n", exp->line);
                            errorCount++;
                        }
                        if (rightType->baseType != TYPE_INT && rightType->baseType != TYPE_FLOAT) {
                            fprintf(stderr, "Error type 7 at Line %d: Type mismatched for operands.\n", exp->line);
                            errorCount++;
                        }
                    }
                }
                break;
            }
            case AND: case OR: {
                if (exp->childCount >= 3) {
                    Type* leftType = getExpressionType(exp->children[0]);
                    Type* rightType = getExpressionType(exp->children[2]);
                    if (leftType != NULL && leftType->baseType != TYPE_INT) {
                        fprintf(stderr, "Error type 7 at Line %d: Type mismatched for operands.\n", exp->line);
                        errorCount++;
                    }
                    if (rightType != NULL && rightType->baseType != TYPE_INT) {
                        fprintf(stderr, "Error type 7 at Line %d: Type mismatched for operands.\n", exp->line);
                        errorCount++;
                    }
                }
                break;
            }
            case NOT: {
                if (exp->childCount >= 2) {
                    Type* leftType = getExpressionType(exp->children[1]);
                    if (leftType != NULL && leftType->baseType != TYPE_INT) {
                        fprintf(stderr, "Error type 7 at Line %d: Type mismatched for operands.\n", exp->line);
                        errorCount++;
                    }
                }
                break;
            }
            case RELOP: {
                if (exp->childCount >= 3) {
                    Type* leftType = getExpressionType(exp->children[0]);
                    Type* rightType = getExpressionType(exp->children[2]);
                    if (leftType != NULL && rightType != NULL && !compareTypes(leftType, rightType)) {
                        fprintf(stderr, "Error type 7 at Line %d: Type mismatched for operands.\n", exp->line);
                        errorCount++;
                    }
                }
                break;
            }
            case DOT: {
                if (exp->childCount >= 3) {
                    Type* leftType = getExpressionType(exp->children[0]);
                    if (leftType == NULL || leftType->baseType != TYPE_STRUCT) {
                        fprintf(stderr, "Error type 13 at Line %d: Illegal use of \".\".\n", exp->line);
                        errorCount++;
                    } else {
                        ASTNode* fieldNode = exp->children[2];
                        if (fieldNode != NULL && fieldNode->tokenType == ID) {
                            Symbol* structSym = lookupSymbol(currentTable, leftType->structName);
                            if (structSym != NULL && structSym->kind == KIND_STRUCT && structSym->type != NULL && structSym->type->fieldTable != NULL) {
                                Symbol* field = lookupSymbol(structSym->type->fieldTable, fieldNode->attr.value);
                                if (field == NULL) {
                                    fprintf(stderr, "Error type 14 at Line %d: Non-existent field \"%s\".\n", exp->line, fieldNode->attr.value);
                                    errorCount++;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case LB: {
                if (exp->childCount >= 3) {
                    Type* leftType = getExpressionType(exp->children[0]);
                    if (leftType == NULL || leftType->baseType != TYPE_ARRAY) {
                        char* exprStr = NULL;
                        ASTNode* left = exp->children[0];
                        if (left != NULL) {
                            if (left->tokenType == ID) {
                                exprStr = left->attr.value;
                            } else if (left->nodeType == NODE_EXP && left->childCount > 0) {
                                ASTNode* child = left->children[0];
                                if (child != NULL && child->tokenType == ID) {
                                    exprStr = child->attr.value;
                                }
                            }
                        }
                        fprintf(stderr, "Error type 10 at Line %d: \"%s\" is not an array.\n", exp->line, 
                            exprStr ? exprStr : "expr");
                        errorCount++;
                    }
                    Type* indexType = getExpressionType(exp->children[2]);
                    if (indexType != NULL && indexType->baseType != TYPE_INT) {
                        char indexStr[32];
                        ASTNode* index = exp->children[2];
                        if (index != NULL) {
                            if (index->tokenType == FLOAT) {
                                sprintf(indexStr, "%g", index->floatValue);
                            } else if (index->tokenType == ID) {
                                strncpy(indexStr, index->attr.value, sizeof(indexStr)-1);
                                indexStr[sizeof(indexStr)-1] = '\0';
                            } else if (index->nodeType == NODE_EXP && index->childCount > 0) {
                                ASTNode* child = index->children[0];
                                if (child != NULL) {
                                    if (child->tokenType == FLOAT) {
                                        sprintf(indexStr, "%g", child->floatValue);
                                    } else if (child->tokenType == ID) {
                                        strncpy(indexStr, child->attr.value, sizeof(indexStr)-1);
                                        indexStr[sizeof(indexStr)-1] = '\0';
                                    } else {
                                        strcpy(indexStr, "expr");
                                    }
                                } else {
                                    strcpy(indexStr, "expr");
                                }
                            } else {
                                strcpy(indexStr, "expr");
                            }
                        } else {
                            strcpy(indexStr, "expr");
                        }
                        fprintf(stderr, "Error type 12 at Line %d: \"%s\" is not an integer.\n", exp->line, indexStr);
                        errorCount++;
                    }
                }
                break;
            }
            case LP: {
                if (exp->childCount >= 1) {
                    ASTNode* idNode = exp->children[0];
                    if (idNode != NULL && idNode->tokenType == ID) {
                        char* funcName = idNode->attr.value;
                        if (strcmp(funcName, "read") == 0 || strcmp(funcName, "write") == 0) {
                            break;
                        }
                        Symbol* sym = lookupSymbol(currentTable, funcName);
                        if (sym == NULL) {
                            fprintf(stderr, "Error type 2 at Line %d: Undefined function \"%s\".\n", exp->line, funcName);
                            errorCount++;
                        } else if (sym->kind != KIND_FUNCTION) {
                            fprintf(stderr, "Error type 11 at Line %d: \"%s\" is not a function.\n", exp->line, idNode->attr.value);
                            errorCount++;
                        } else {
                            int argCount = 0;
                            ASTNode* args = NULL;
                            if (exp->childCount > 2) {
                                args = exp->children[2];
                            }
                            if (args != NULL && args->nodeType == NODE_ARGS) {
                                ASTNode* cur = args;
                                while (cur != NULL && cur->childCount > 0) {
                                    argCount++;
                                    if (cur->childCount >= 3) {
                                        cur = cur->children[2];
                                    } else {
                                        break;
                                    }
                                }
                            }
                            if (argCount != sym->paramCount) {
                                fprintf(stderr, "Error type 9 at Line %d: Function \"%s(int)\" is not applicable for arguments \"(int, int)\".\n", 
                                    exp->line, idNode->attr.value);
                                errorCount++;
                            } else {
                                ASTNode* args2 = NULL;
                                if (exp->childCount > 2) {
                                    args2 = exp->children[2];
                                }
                                if (args2 != NULL && args2->nodeType == NODE_ARGS) {
                                    ASTNode* cur = args2;
                                    int paramIdx = 0;
                                    while (cur != NULL && cur->childCount > 0 && paramIdx < sym->paramCount) {
                                        ASTNode* argExp = cur->children[0];
                                        Type* argType = getExpressionType(argExp);
                                        Type* paramType = sym->paramTypes[paramIdx];
                                        if (argType != NULL && paramType != NULL && !compareTypes(argType, paramType)) {
                                            fprintf(stderr, "Error type 9 at Line %d: Function \"%s(int)\" is not applicable for arguments.\n", 
                                                exp->line, idNode->attr.value);
                                            errorCount++;
                                            break;
                                        }
                                        paramIdx++;
                                        if (cur->childCount >= 3) {
                                            cur = cur->children[2];
                                        } else {
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            default: break;
        }
    }
}

static void traverseStmt(ASTNode* stmt) {
    if (stmt == NULL) return;
    if (stmt->nodeType == NODE_TOKEN) return;
    
    if (stmt->nodeType == NODE_STMT) {
        for (int i = 0; i < stmt->childCount; i++) {
            ASTNode* child = stmt->children[i];
            if (child != NULL) {
                if (child->tokenType == RETURN) {
                    if (inFunction && currentReturnType != NULL && stmt->childCount > 1) {
                        Type* expType = getExpressionType(stmt->children[1]);
                        if (expType != NULL && !compareTypes(expType, currentReturnType)) {
                            fprintf(stderr, "Error type 8 at Line %d: Type mismatched for return.\n", stmt->line);
                            errorCount++;
                        }
                    }
                }
                traverseStmt(child);
                traverseDef(child);
            }
        }
    }
    if (stmt->nodeType == NODE_COMPST) {
        if (stmt->childCount > 1) {
            traverseDef(stmt->children[1]);
        }
        if (stmt->childCount > 2) {
            traverseStmt(stmt->children[2]);
        }
    }
    if (stmt->nodeType == NODE_STMTLIST) {
        traverseStmt(stmt->children[0]);
        if (stmt->childCount > 1) {
            traverseStmt(stmt->children[1]);
        }
    }
    if (stmt->nodeType == NODE_EXP) {
        traverseExp(stmt);
    }
}

static void processDecList(ASTNode* decList, Type* baseType, int line) {
    if (decList == NULL) return;
    
    if (decList->nodeType == NODE_DECLIST) {
        for (int i = 0; i < decList->childCount; i += 2) {
            ASTNode* dec = decList->children[i];
            if (dec != NULL) {
                if (dec->nodeType == NODE_DEC) {
                    ASTNode* varDec = dec->children[0];
                    char* name = getVarDecName(varDec);
                    if (name != NULL) {
                        Symbol* existing = lookupSymbol(currentTable, name);
                        if (existing != NULL) {
                            fprintf(stderr, "Error type 3 at Line %d: Redefined variable \"%s\".\n", line, name);
                            errorCount++;
                        } else {
                            Symbol* structSym = lookupSymbol(currentTable, name);
                            if (structSym != NULL && structSym->kind == KIND_STRUCT) {
                                fprintf(stderr, "Error type 3 at Line %d: Redefined variable \"%s\".\n", line, name);
                                errorCount++;
                            } else {
                                Type* varType = getTypeFromVarDec(varDec, baseType);
                                Symbol* sym = createSymbol(name, KIND_VARIABLE, varType, line);
                                insertSymbol(currentTable, sym);
                            }
                        }
                    }
                } else if (dec->nodeType == NODE_DECLIST) {
                    processDecList(dec, baseType, line);
                }
            }
        }
    }
}

static void traverseDef(ASTNode* def) {
    if (def == NULL) return;
    if (def->nodeType == NODE_DEF) {
        ASTNode* specifier = NULL;
        ASTNode* decList = NULL;
        if (def->childCount > 0) {
            specifier = def->children[0];
        }
        if (def->childCount > 1) {
            decList = def->children[1];
        }
        Type* baseType = getTypeFromSpecifier(specifier);
        if (decList != NULL) {
            processDecList(decList, baseType, def->line);
        }
    }
    if (def->nodeType == NODE_DEFLIST) {
        traverseDef(def->children[0]);
        if (def->childCount > 1) {
            traverseDef(def->children[1]);
        }
    }
}

static void collectParamTypes(ASTNode* varList, Symbol* funcSym) {
    if (varList == NULL || funcSym == NULL) return;
    
    if (varList->nodeType == NODE_VARLIST) {
        ASTNode* paramDec = NULL;
        if (varList->childCount > 0) {
            paramDec = varList->children[0];
        }
        if (paramDec != NULL && paramDec->nodeType == NODE_PARAMDEC) {
            ASTNode* specifier = NULL;
            ASTNode* varDec = NULL;
            if (paramDec->childCount > 0) {
                specifier = paramDec->children[0];
            }
            if (paramDec->childCount > 1) {
                varDec = paramDec->children[1];
            }
            Type* paramType = getTypeFromSpecifier(specifier);
            char* name = getVarDecName(varDec);
            if (name != NULL) {
                Type* fullType = getTypeFromVarDec(varDec, paramType);
                funcSym->paramTypes[funcSym->paramCount] = fullType;
                funcSym->paramCount++;
            }
        }
        if (varList->childCount >= 3) {
            collectParamTypes(varList->children[2], funcSym);
        }
    }
}

static void collectParams(ASTNode* varList, Symbol* funcSym) {
    if (varList == NULL || funcSym == NULL) return;
    
    if (varList->nodeType == NODE_VARLIST) {
        ASTNode* paramDec = NULL;
        if (varList->childCount > 0) {
            paramDec = varList->children[0];
        }
        if (paramDec != NULL && paramDec->nodeType == NODE_PARAMDEC) {
            ASTNode* specifier = NULL;
            ASTNode* varDec = NULL;
            if (paramDec->childCount > 0) {
                specifier = paramDec->children[0];
            }
            if (paramDec->childCount > 1) {
                varDec = paramDec->children[1];
            }
            Type* paramType = getTypeFromSpecifier(specifier);
            char* name = getVarDecName(varDec);
            if (name != NULL) {
                Type* fullType = getTypeFromVarDec(varDec, paramType);
                Symbol* paramSym = createSymbol(name, KIND_PARAMETER, fullType, paramDec->line);
                insertSymbol(currentTable, paramSym);
            }
        }
        if (varList->childCount >= 3) {
            collectParams(varList->children[2], funcSym);
        }
    }
}

static void traverseExtDef(ASTNode* extDef) {
    if (extDef == NULL) return;
    if (extDef->nodeType == NODE_EXTDEF) {
        ASTNode* specifier = NULL;
        ASTNode* extDecList = NULL;
        if (extDef->childCount > 0) {
            specifier = extDef->children[0];
        }
        if (extDef->childCount > 1) {
            extDecList = extDef->children[1];
        }
        
        if (specifier != NULL && specifier->nodeType == NODE_STRUCTSPECIFIER) {
            ASTNode* optTag = NULL;
            ASTNode* lc = NULL;
            ASTNode* defList = NULL;
            if (specifier->childCount > 1) {
                optTag = specifier->children[1];
            }
            if (specifier->childCount > 2) {
                lc = specifier->children[2];
            }
            if (specifier->childCount > 3) {
                defList = specifier->children[3];
            }
            
            if (optTag != NULL && optTag->nodeType == NODE_OPTTAG && optTag->childCount > 0) {
                char* name = optTag->children[0]->attr.value;
                Symbol* existing = lookupSymbol(currentTable, name);
                if (existing != NULL) {
                    fprintf(stderr, "Error type 16 at Line %d: Duplicated name \"%s\".\n", extDef->line, name);
                    errorCount++;
                } else {
                    Type* structType = createStructType(name);
                    Symbol* structSym = createSymbol(name, KIND_STRUCT, structType, extDef->line);
                    insertSymbol(currentTable, structSym);
                    
                    if (lc != NULL && defList != NULL) {
                        processStructFields(defList, structSym, extDef->line);
                    }
                }
            }
            return;
        }
        
        if (extDecList != NULL) {
            if (extDecList->nodeType == NODE_FUNDEC) {
                ASTNode* idNode = NULL;
                ASTNode* varList = NULL;
                if (extDecList->childCount > 0) {
                    idNode = extDecList->children[0];
                }
                if (extDecList->childCount > 2) {
                    varList = extDecList->children[2];
                }
                int isDecl = 0;
                if (extDef->childCount >= 3) {
                    ASTNode* lastChild = extDef->children[2];
                    if (lastChild != NULL && lastChild->nodeType == NODE_TOKEN && lastChild->tokenType == SEMI) {
                        isDecl = 1;
                    }
                }
                
                if (idNode != NULL && idNode->tokenType == ID) {
                    char* name = idNode->attr.value;
                    Type* returnType = getTypeFromSpecifier(specifier);
                    Symbol* existing = lookupSymbol(currentTable, name);
                    
                    Type* funcType = createFunctionType(returnType, 0, NULL);
                    Symbol* funcSym = createSymbol(name, KIND_FUNCTION, funcType, extDef->line);
                    funcSym->isDeclaration = isDecl ? 1 : 0;
                    if (varList != NULL && varList->nodeType == NODE_VARLIST) {
                        collectParamTypes(varList, funcSym);
                    }
                    
                    if (isDecl) {
                        if (existing != NULL && existing->kind == KIND_FUNCTION) {
                            if (!existing->isDeclaration) {
                                fprintf(stderr, "Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n", extDef->line, name);
                                errorCount++;
                            } else {
                                int conflict = 0;
                                if (!compareTypes(existing->type, funcType)) conflict = 1;
                                if (existing->paramCount != funcSym->paramCount) conflict = 1;
                                if (!conflict) {
                                    for (int i = 0; i < existing->paramCount; i++) {
                                        if (!compareTypes(existing->paramTypes[i], funcSym->paramTypes[i])) {
                                            conflict = 1;
                                            break;
                                        }
                                    }
                                }
                                if (conflict) {
                                    fprintf(stderr, "Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n", extDef->line, name);
                                    errorCount++;
                                }
                            }
                        } else {
                            insertSymbol(currentTable, funcSym);
                        }
                    } else {
                        if (existing != NULL && existing->kind == KIND_FUNCTION) {
                            if (!existing->isDeclaration) {
                                fprintf(stderr, "Error type 4 at Line %d: Redefined function \"%s\".\n", extDef->line, name);
                                errorCount++;
                            } else {
                                int conflict = 0;
                                if (!compareTypes(existing->type, funcType)) conflict = 1;
                                if (existing->paramCount != funcSym->paramCount) conflict = 1;
                                if (!conflict) {
                                    for (int i = 0; i < existing->paramCount; i++) {
                                        if (!compareTypes(existing->paramTypes[i], funcSym->paramTypes[i])) {
                                            conflict = 1;
                                            break;
                                        }
                                    }
                                }
                                if (conflict) {
                                    fprintf(stderr, "Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n", extDef->line, name);
                                    errorCount++;
                                }
                                existing->isDeclaration = 0;
                                
                                Type* savedReturnType = currentReturnType;
                                int savedInFunction = inFunction;
                                SymbolTable* savedTable = currentTable;
                                currentReturnType = returnType;
                                inFunction = 1;
                                currentTable = createSymbolTable(savedTable);
                                
                                if (varList != NULL && varList->nodeType == NODE_VARLIST) {
                                    collectParams(varList, funcSym);
                                }
                                
                                if (extDef->childCount >= 3 && extDef->children[2] != NULL && extDef->children[2]->nodeType == NODE_COMPST) {
                                    traverseStmt(extDef->children[2]);
                                }
                                
                                currentTable = savedTable;
                                currentReturnType = savedReturnType;
                                inFunction = savedInFunction;
                            }
                        } else if (existing != NULL && existing->kind != KIND_FUNCTION) {
                            fprintf(stderr, "Error type 4 at Line %d: Redefined function \"%s\".\n", extDef->line, name);
                            errorCount++;
                        } else {
                            insertSymbol(currentTable, funcSym);
                            
                            Type* savedReturnType = currentReturnType;
                            int savedInFunction = inFunction;
                            SymbolTable* savedTable = currentTable;
                            currentReturnType = returnType;
                            inFunction = 1;
                            currentTable = createSymbolTable(savedTable);
                                
                                if (varList != NULL && varList->nodeType == NODE_VARLIST) {
                                    collectParams(varList, funcSym);
                                }
                                
                                if (extDef->childCount >= 3 && extDef->children[2] != NULL && extDef->children[2]->nodeType == NODE_COMPST) {
                                    traverseStmt(extDef->children[2]);
                                }
                                
                                currentTable = savedTable;
                            currentReturnType = savedReturnType;
                            inFunction = savedInFunction;
                        }
                    }
                }
            } else if (extDecList->nodeType == NODE_EXTDECLIST || extDecList->nodeType == NODE_VARDEC) {
                Type* baseType = getTypeFromSpecifier(specifier);
                if (extDecList->nodeType == NODE_VARDEC) {
                    char* name = getVarDecName(extDecList);
                    if (name != NULL) {
                        Symbol* existing = lookupSymbol(currentTable, name);
                        if (existing != NULL) {
                            fprintf(stderr, "Error type 3 at Line %d: Redefined variable \"%s\".\n", extDef->line, name);
                            errorCount++;
                        } else {
                            Type* varType = getTypeFromVarDec(extDecList, baseType);
                            Symbol* sym = createSymbol(name, KIND_VARIABLE, varType, extDef->line);
                            insertSymbol(currentTable, sym);
                        }
                    }
                } else {
                    for (int i = 0; i < extDecList->childCount; i += 2) {
                        ASTNode* varDec = extDecList->children[i];
                        char* name = getVarDecName(varDec);
                        if (name != NULL) {
                            Symbol* existing = lookupSymbol(currentTable, name);
                            if (existing != NULL) {
                                fprintf(stderr, "Error type 3 at Line %d: Redefined variable \"%s\".\n", extDef->line, name);
                                errorCount++;
                            } else {
                                Type* varType = getTypeFromVarDec(varDec, baseType);
                                Symbol* sym = createSymbol(name, KIND_VARIABLE, varType, extDef->line);
                                insertSymbol(currentTable, sym);
                            }
                        }
                    }
                }
            }
        }
    }
}

static void traverseExtDefList(ASTNode* extDefList) {
    if (extDefList == NULL) return;
    if (extDefList->nodeType == NODE_EXTDEFLIST) {
        traverseExtDef(extDefList->children[0]);
        if (extDefList->childCount > 1) {
            traverseExtDefList(extDefList->children[1]);
        }
    }
}

void semanticAnalysis(ASTNode* root) {
    currentTable = createSymbolTable(NULL);
    currentReturnType = NULL;
    inFunction = 0;
    if (root != NULL && root->nodeType == NODE_PROGRAM) {
        traverseExtDefList(root->children[0]);
    }
    
    Symbol* s = currentTable->head;
    while (s != NULL) {
        if (s->kind == KIND_FUNCTION && s->isDeclaration) {
            fprintf(stderr, "Error type 18 at Line %d: Undefined function \"%s\".\n", s->line, s->name);
            errorCount++;
        }
        s = s->next;
    }
}
