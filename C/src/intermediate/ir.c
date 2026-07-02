#include "ir.h"

IRNode* irList = NULL;
int tempCount = 0;
int labelCount = 0;
int varCount = 1;

#define MAX_VARS 100
char* varNames[MAX_VARS];
char* varMap[MAX_VARS];

static void* safeMalloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    return ptr;
}

static char* getVarName(const char* name) {
    for (int i = 0; i < varCount; i++) {
        if (varNames[i] != NULL && strcmp(varNames[i], name) == 0) {
            return varMap[i];
        }
    }
    if (varCount >= MAX_VARS) return strdup(name);
    varNames[varCount-1] = strdup(name);
    varMap[varCount-1] = (char*)safeMalloc(20);
    sprintf(varMap[varCount-1], "v%d", varCount);
    varCount++;
    return varMap[varCount-2];
}

static char* genExpInternal(ASTNode* node, int isCondition);

IRNode* createIRNode(IRType type) {
    IRNode* node = (IRNode*)safeMalloc(sizeof(IRNode));
    node->type = type;
    node->result = NULL;
    node->arg1 = NULL;
    node->arg2 = NULL;
    node->arg3 = NULL;
    node->next = NULL;
    return node;
}

void addIRNode(IRNode* node) {
    if (irList == NULL) {
        irList = node;
    } else {
        IRNode* p = irList;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = node;
    }
}

char* newTemp() {
    char* temp = (char*)safeMalloc(20);
    sprintf(temp, "t%d", ++tempCount);
    return temp;
}

char* newLabel() {
    char* label = (char*)safeMalloc(20);
    sprintf(label, "label%d", ++labelCount);
    return label;
}

void printIR(FILE* fp) {
    IRNode* p = irList;
    while (p != NULL) {
        switch (p->type) {
            case IR_LABEL:
                fprintf(fp, "LABEL %s:\n", p->result ? p->result : "");
                break;
            case IR_FUNCTION:
                fprintf(fp, "FUNCTION %s:\n", p->result ? p->result : "");
                break;
            case IR_ASSIGN:
                if (p->arg2 != NULL) {
                    fprintf(fp, "%s := %s\n", p->result ? p->result : "", p->arg2);
                } else if (p->arg1 != NULL) {
                    fprintf(fp, "%s := %s\n", p->result ? p->result : "", p->arg1);
                } else {
                    fprintf(fp, "%s :=\n", p->result ? p->result : "");
                }
                break;
            case IR_ADD:
                fprintf(fp, "%s := %s + %s\n", p->result ? p->result : "", p->arg1 ? p->arg1 : "", p->arg2 ? p->arg2 : "");
                break;
            case IR_SUB:
                fprintf(fp, "%s := %s - %s\n", p->result ? p->result : "", p->arg1 ? p->arg1 : "", p->arg2 ? p->arg2 : "");
                break;
            case IR_MUL:
                fprintf(fp, "%s := %s * %s\n", p->result ? p->result : "", p->arg1 ? p->arg1 : "", p->arg2 ? p->arg2 : "");
                break;
            case IR_DIV:
                fprintf(fp, "%s := %s / %s\n", p->result ? p->result : "", p->arg1 ? p->arg1 : "", p->arg2 ? p->arg2 : "");
                break;
            case IR_ADDR:
                fprintf(fp, "%s := &%s\n", p->result ? p->result : "", p->arg1 ? p->arg1 : "");
                break;
            case IR_DEREF:
                fprintf(fp, "%s := *%s\n", p->result ? p->result : "", p->arg1 ? p->arg1 : "");
                break;
            case IR_DEREF_ASSIGN:
                fprintf(fp, "*%s := %s\n", p->result ? p->result : "", p->arg1 ? p->arg1 : "");
                break;
            case IR_GOTO:
                fprintf(fp, "GOTO %s\n", p->result ? p->result : "");
                break;
            case IR_IF: {
                const char* relopStr;
                switch (p->relop) {
                    case RELOP_EQ: relopStr = "=="; break;
                    case RELOP_NE: relopStr = "!="; break;
                    case RELOP_LT: relopStr = "<"; break;
                    case RELOP_GT: relopStr = ">"; break;
                    case RELOP_LE: relopStr = "<="; break;
                    case RELOP_GE: relopStr = ">="; break;
                    default: relopStr = "==";
                }
                fprintf(fp, "IF %s %s %s GOTO %s\n", p->arg1 ? p->arg1 : "", relopStr, p->arg2 ? p->arg2 : "", p->result ? p->result : "");
                break;
            }
            case IR_RETURN:
                fprintf(fp, "RETURN %s\n", p->arg1 ? p->arg1 : "");
                break;
            case IR_DEC:
                fprintf(fp, "DEC %s %s\n", p->result ? p->result : "", p->arg1 ? p->arg1 : "");
                break;
            case IR_ARG:
                fprintf(fp, "ARG %s\n", p->arg1 ? p->arg1 : "");
                break;
            case IR_CALL:
                fprintf(fp, "%s := CALL %s\n", p->result ? p->result : "", p->arg1 ? p->arg1 : "");
                break;
            case IR_PARAM:
                fprintf(fp, "PARAM %s\n", p->arg1 ? p->arg1 : "");
                break;
            case IR_READ:
                fprintf(fp, "READ %s\n", p->arg1 ? p->arg1 : "");
                break;
            case IR_WRITE:
                fprintf(fp, "WRITE %s\n", p->arg1 ? p->arg1 : "");
                break;
        }
        p = p->next;
    }
}

void freeIR() {
    IRNode* p = irList;
    while (p != NULL) {
        IRNode* next = p->next;
        if (p->result) free(p->result);
        if (p->arg1) free(p->arg1);
        if (p->arg2) free(p->arg2);
        if (p->arg3) free(p->arg3);
        free(p);
        p = next;
    }
    irList = NULL;
}

void genIR(ASTNode* root) {
    tempCount = 0;
    labelCount = 0;
    irList = NULL;
    
    if (root != NULL && root->nodeType == NODE_PROGRAM && root->childCount > 0) {
        genExtDefList(root->children[0]);
    }
}

void genExtDefList(ASTNode* node) {
    if (node == NULL) return;
    
    if (node->nodeType == NODE_EXTDEFLIST) {
        if (node->childCount > 0) {
            genExtDef(node->children[0]);
            if (node->childCount > 1) {
                genExtDefList(node->children[1]);
            }
        }
    }
}

void genExtDef(ASTNode* node) {
    if (node == NULL) return;
    
    if (node->nodeType == NODE_EXTDEF) {
        ASTNode* second = NULL;
        
        if (node->childCount > 1) {
            second = node->children[1];
        }
        
        if (second != NULL && second->nodeType == NODE_FUNDEC) {
            ASTNode* idNode = NULL;
            if (second->childCount > 0) {
                idNode = second->children[0];
            }
            
            ASTNode* varList = NULL;
            if (second->childCount > 2 && second->children[2]->nodeType == NODE_VARLIST) {
                varList = second->children[2];
            }
            
            ASTNode* compSt = NULL;
            if (node->childCount > 2) {
                compSt = node->children[2];
            }
            
            if (idNode != NULL && idNode->tokenType == ID) {
                char* funcName = idNode->attr.value;
                
                IRNode* funcIR = createIRNode(IR_FUNCTION);
                funcIR->result = strdup(funcName);
                addIRNode(funcIR);
                
                if (varList != NULL && varList->nodeType == NODE_VARLIST) {
                    ASTNode* vl = varList;
                    while (vl != NULL) {
                        if (vl->nodeType == NODE_VARLIST && vl->childCount > 0) {
                            ASTNode* param = vl->children[0];
                            if (param != NULL && param->nodeType == NODE_PARAMDEC && param->childCount > 1) {
                                ASTNode* paramVarDec = param->children[1];
                                if (paramVarDec != NULL && paramVarDec->nodeType == NODE_VARDEC && paramVarDec->childCount > 0) {
                                    ASTNode* paramId = paramVarDec->children[0];
                                    if (paramId != NULL && paramId->tokenType == ID) {
                                        IRNode* paramIR = createIRNode(IR_PARAM);
                                        paramIR->arg1 = strdup(getVarName(paramId->attr.value));
                                        addIRNode(paramIR);
                                    }
                                }
                            }
                            if (vl->childCount > 2) {
                                vl = vl->children[2];
                            } else {
                                break;
                            }
                        } else {
                            break;
                        }
                    }
                }
                
                if (compSt != NULL && compSt->nodeType == NODE_COMPST) {
                    genCompSt(compSt);
                }
            }
        } else if (second != NULL && second->nodeType == NODE_EXTDECLIST) {
            genDecList(second);
        }
    }
}

char* genExp(ASTNode* node) {
    return genExpInternal(node, 0);
}

char* genExpInternal(ASTNode* node, int isCondition) {
    if (node == NULL) return NULL;
    
    if (node->nodeType == NODE_EXP) {
        if (node->childCount == 1) {
            ASTNode* child = node->children[0];
            if (child != NULL) {
                if (child->tokenType == ID) {
                    return strdup(getVarName(child->attr.value));
                }
                if (child->tokenType == INT) {
                    char* result = (char*)safeMalloc(20);
                    sprintf(result, "#%d", child->intValue);
                    return result;
                }
                if (child->tokenType == FLOAT) {
                    char* result = (char*)safeMalloc(20);
                    sprintf(result, "#%.6f", child->floatValue);
                    return result;
                }
                if (child->nodeType == NODE_EXP) {
                    return genExpInternal(child, isCondition);
                }
            }
            return NULL;
        }
        
        if (node->childCount < 1) return NULL;
        ASTNode* left = node->children[0];
        
        if (left == NULL) return NULL;
        
        if (left->nodeType == NODE_EXP) {
            if (node->childCount < 3) return NULL;
            ASTNode* op = node->children[1];
            ASTNode* right = node->children[2];
            
            if (op == NULL || right == NULL) return NULL;
            
            int tokenType = op->tokenType;
            
            char* leftVal = genExpInternal(left, isCondition);
            char* rightVal = genExpInternal(right, isCondition);
            
            if (leftVal == NULL || rightVal == NULL) return NULL;
            
            IRNode* ir = NULL;
            
            switch (tokenType) {
                case PLUS: {
                    char* result = newTemp();
                    ir = createIRNode(IR_ADD);
                    ir->result = strdup(result);
                    ir->arg1 = strdup(leftVal);
                    ir->arg2 = strdup(rightVal);
                    addIRNode(ir);
                    return result;
                }
                case MINUS: {
                    char* result = newTemp();
                    ir = createIRNode(IR_SUB);
                    ir->result = strdup(result);
                    ir->arg1 = strdup(leftVal);
                    ir->arg2 = strdup(rightVal);
                    addIRNode(ir);
                    return result;
                }
                case STAR: {
                    char* result = newTemp();
                    ir = createIRNode(IR_MUL);
                    ir->result = strdup(result);
                    ir->arg1 = strdup(leftVal);
                    ir->arg2 = strdup(rightVal);
                    addIRNode(ir);
                    return result;
                }
                case DIV: {
                    char* result = newTemp();
                    ir = createIRNode(IR_DIV);
                    ir->result = strdup(result);
                    ir->arg1 = strdup(leftVal);
                    ir->arg2 = strdup(rightVal);
                    addIRNode(ir);
                    return result;
                }
                case ASSIGNOP: {
                    IRNode* assignIR = createIRNode(IR_ASSIGN);
                    assignIR->result = strdup(leftVal);
                    assignIR->arg1 = strdup(rightVal);
                    addIRNode(assignIR);
                    
                    return leftVal;
                }
                case AND:
                case OR: {
                    char* result = newTemp();
                    char* labelTrue = newLabel();
                    char* labelFalse = newLabel();
                    char* labelEnd = newLabel();
                    
                    IRNode* ifIR1 = createIRNode(IR_IF);
                    ifIR1->arg1 = strdup(leftVal);
                    ifIR1->arg2 = strdup("#0");
                    ifIR1->relop = tokenType == AND ? RELOP_NE : RELOP_EQ;
                    ifIR1->result = strdup(labelTrue);
                    addIRNode(ifIR1);
                    
                    IRNode* gotoIR1 = createIRNode(IR_GOTO);
                    gotoIR1->result = strdup(labelFalse);
                    addIRNode(gotoIR1);
                    
                    IRNode* labelIR1 = createIRNode(IR_LABEL);
                    labelIR1->result = strdup(labelTrue);
                    addIRNode(labelIR1);
                    
                    IRNode* ifIR2 = createIRNode(IR_IF);
                    ifIR2->arg1 = strdup(rightVal);
                    ifIR2->arg2 = strdup("#0");
                    ifIR2->relop = tokenType == AND ? RELOP_NE : RELOP_EQ;
                    ifIR2->result = strdup(labelEnd);
                    addIRNode(ifIR2);
                    
                    IRNode* gotoIR2 = createIRNode(IR_GOTO);
                    gotoIR2->result = strdup(labelFalse);
                    addIRNode(gotoIR2);
                    
                    IRNode* labelIR2 = createIRNode(IR_LABEL);
                    labelIR2->result = strdup(labelFalse);
                    addIRNode(labelIR2);
                    
                    IRNode* assignIR1 = createIRNode(IR_ASSIGN);
                    assignIR1->result = strdup(result);
                    assignIR1->arg2 = strdup("#0");
                    addIRNode(assignIR1);
                    
                    IRNode* gotoIR3 = createIRNode(IR_GOTO);
                    gotoIR3->result = strdup(labelEnd);
                    addIRNode(gotoIR3);
                    
                    IRNode* labelIR3 = createIRNode(IR_LABEL);
                    labelIR3->result = strdup(labelEnd);
                    addIRNode(labelIR3);
                    
                    IRNode* assignIR2 = createIRNode(IR_ASSIGN);
                    assignIR2->result = strdup(result);
                    assignIR2->arg2 = strdup("#1");
                    addIRNode(assignIR2);
                    
                    return result;
                }
                case RELOP: {
                    if (isCondition) {
                        char* result = newTemp();
                        result[0] = '\0';
                        return result;
                    }
                    
                    char* result = newTemp();
                    char* labelTrue = newLabel();
                    char* labelFalse = newLabel();
                    char* labelEnd = newLabel();
                    
                    ASTNode* op2 = node->children[1];
                    char* relopVal = op2->attr.value;
                    RelOp relop;
                    if (strcmp(relopVal, "<") == 0) relop = RELOP_LT;
                    else if (strcmp(relopVal, ">") == 0) relop = RELOP_GT;
                    else if (strcmp(relopVal, "<=") == 0) relop = RELOP_LE;
                    else if (strcmp(relopVal, ">=") == 0) relop = RELOP_GE;
                    else if (strcmp(relopVal, "==") == 0) relop = RELOP_EQ;
                    else relop = RELOP_NE;
                    
                    IRNode* ifIR = createIRNode(IR_IF);
                    ifIR->arg1 = strdup(leftVal);
                    ifIR->arg2 = strdup(rightVal);
                    ifIR->relop = relop;
                    ifIR->result = strdup(labelTrue);
                    addIRNode(ifIR);
                    
                    IRNode* gotoIR1 = createIRNode(IR_GOTO);
                    gotoIR1->result = strdup(labelFalse);
                    addIRNode(gotoIR1);
                    
                    IRNode* labelIR1 = createIRNode(IR_LABEL);
                    labelIR1->result = strdup(labelTrue);
                    addIRNode(labelIR1);
                    
                    IRNode* assignIR1 = createIRNode(IR_ASSIGN);
                    assignIR1->result = strdup(result);
                    assignIR1->arg2 = strdup("#1");
                    addIRNode(assignIR1);
                    
                    IRNode* gotoIR2 = createIRNode(IR_GOTO);
                    gotoIR2->result = strdup(labelEnd);
                    addIRNode(gotoIR2);
                    
                    IRNode* labelIR2 = createIRNode(IR_LABEL);
                    labelIR2->result = strdup(labelFalse);
                    addIRNode(labelIR2);
                    
                    IRNode* assignIR2 = createIRNode(IR_ASSIGN);
                    assignIR2->result = strdup(result);
                    assignIR2->arg2 = strdup("#0");
                    addIRNode(assignIR2);
                    
                    IRNode* labelIR3 = createIRNode(IR_LABEL);
                    labelIR3->result = strdup(labelEnd);
                    addIRNode(labelIR3);
                    
                    return result;
                }
                default:
                    return leftVal;
            }
        }
        
        if (left->tokenType == ID) {
            if (node->childCount < 2) return strdup(left->attr.value);
            ASTNode* op = node->children[1];
            if (op != NULL) {
                if (op->tokenType == DOT) {
                    return NULL;
                }
                
                if (op->tokenType == LB) {
                    return NULL;
                }
                
                if (op->tokenType == LP) {
                    char* funcName = left->attr.value;
                    ASTNode* args = NULL;
                    if (node->childCount > 2) {
                        args = node->children[2];
                    }
                    
                    if (strcmp(funcName, "read") == 0) {
                        char* result = newTemp();
                        IRNode* readIR = createIRNode(IR_READ);
                        readIR->arg1 = strdup(result);
                        addIRNode(readIR);
                        return result;
                    }
                    
                    if (strcmp(funcName, "write") == 0) {
                        if (args != NULL && args->nodeType == NODE_ARGS && args->childCount > 0) {
                            char* argVal = genExpInternal(args->children[0], 0);
                            if (argVal != NULL && argVal[0] == '#') {
                                char* tempVar = newTemp();
                                IRNode* assignIR = createIRNode(IR_ASSIGN);
                                assignIR->result = strdup(tempVar);
                                assignIR->arg2 = strdup(argVal);
                                addIRNode(assignIR);
                                
                                IRNode* writeIR = createIRNode(IR_WRITE);
                                writeIR->arg1 = strdup(tempVar);
                                addIRNode(writeIR);
                            } else {
                                IRNode* writeIR = createIRNode(IR_WRITE);
                                writeIR->arg1 = argVal ? strdup(argVal) : strdup("#0");
                                addIRNode(writeIR);
                            }
                        }
                        return strdup("#0");
                    }
                    
                    if (args != NULL && args->nodeType == NODE_ARGS) {
                        ASTNode* argList = args;
                        while (argList != NULL && argList->nodeType == NODE_ARGS) {
                            ASTNode* arg = argList->children[0];
                            if (arg != NULL) {
                                char* argVal = genExpInternal(arg, 0);
                                if (argVal != NULL) {
                                    IRNode* argIR = createIRNode(IR_ARG);
                                    argIR->arg1 = strdup(argVal);
                                    addIRNode(argIR);
                                }
                            }
                            if (argList->childCount > 2) {
                                argList = argList->children[2];
                            } else {
                                break;
                            }
                        }
                    }
                    
                    char* result = newTemp();
                    IRNode* callIR = createIRNode(IR_CALL);
                    callIR->result = strdup(result);
                    callIR->arg1 = strdup(funcName);
                    addIRNode(callIR);
                    
                    return result;
                }
            }
            
            return strdup(left->attr.value);
        }
        
        if (left->tokenType == INT) {
            char* result = (char*)safeMalloc(20);
            sprintf(result, "#%d", left->intValue);
            return result;
        }
        
        if (left->tokenType == FLOAT) {
            char* result = (char*)safeMalloc(20);
            sprintf(result, "#%.6f", left->floatValue);
            return result;
        }
        
        if (left->tokenType == LP) {
            if (node->childCount > 1) {
                return genExpInternal(node->children[1], isCondition);
            }
        }
        
        if (left->tokenType == MINUS) {
            if (node->childCount < 2) return NULL;
            char* rightVal = genExpInternal(node->children[1], isCondition);
            if (rightVal != NULL) {
                char* tempRight = rightVal;
                if (rightVal[0] == '#') {
                    tempRight = newTemp();
                    IRNode* assignIR = createIRNode(IR_ASSIGN);
                    assignIR->result = strdup(tempRight);
                    assignIR->arg2 = strdup(rightVal);
                    addIRNode(assignIR);
                }
                
                char* result = newTemp();
                IRNode* subIR = createIRNode(IR_SUB);
                subIR->result = strdup(result);
                subIR->arg1 = strdup("#0");
                subIR->arg2 = strdup(tempRight);
                addIRNode(subIR);
                return result;
            }
        }
        
        if (left->tokenType == NOT) {
            if (node->childCount < 2) return NULL;
            char* rightVal = genExpInternal(node->children[1], isCondition);
            if (rightVal != NULL) {
                char* result = newTemp();
                char* labelTrue = newLabel();
                char* labelFalse = newLabel();
                char* labelEnd = newLabel();
                
                IRNode* ifIR = createIRNode(IR_IF);
                ifIR->arg1 = strdup(rightVal);
                ifIR->arg2 = strdup("#0");
                ifIR->relop = RELOP_EQ;
                ifIR->result = strdup(labelTrue);
                addIRNode(ifIR);
                
                IRNode* gotoIR1 = createIRNode(IR_GOTO);
                gotoIR1->result = strdup(labelFalse);
                addIRNode(gotoIR1);
                
                IRNode* labelIR1 = createIRNode(IR_LABEL);
                labelIR1->result = strdup(labelTrue);
                addIRNode(labelIR1);
                
                IRNode* assignIR1 = createIRNode(IR_ASSIGN);
                assignIR1->result = strdup(result);
                assignIR1->arg2 = strdup("#1");
                addIRNode(assignIR1);
                
                IRNode* gotoIR2 = createIRNode(IR_GOTO);
                gotoIR2->result = strdup(labelEnd);
                addIRNode(gotoIR2);
                
                IRNode* labelIR2 = createIRNode(IR_LABEL);
                labelIR2->result = strdup(labelFalse);
                addIRNode(labelIR2);
                
                IRNode* assignIR2 = createIRNode(IR_ASSIGN);
                assignIR2->result = strdup(result);
                assignIR2->arg2 = strdup("#0");
                addIRNode(assignIR2);
                
                IRNode* labelIR3 = createIRNode(IR_LABEL);
                labelIR3->result = strdup(labelEnd);
                addIRNode(labelIR3);
                
                return result;
            }
        }
    } else if (node->nodeType == NODE_TOKEN) {
        if (node->tokenType == ID) {
            return strdup(node->attr.value);
        }
        if (node->tokenType == INT) {
            char* result = (char*)safeMalloc(20);
            sprintf(result, "#%d", node->intValue);
            return result;
        }
        if (node->tokenType == FLOAT) {
            char* result = (char*)safeMalloc(20);
            sprintf(result, "#%.6f", node->floatValue);
            return result;
        }
    }
    
    return NULL;
}

void genStmt(ASTNode* node) {
    if (node == NULL) return;
    
    if (node->nodeType == NODE_STMT) {
        if (node->childCount == 0) return;
        
        ASTNode* child = node->children[0];
        
        if (child == NULL) return;
        
        if (child->tokenType == IF) {
            char* labelThen = newLabel();
            char* labelElse = newLabel();
            char* labelEnd = newLabel();
            
            ASTNode* condExp = NULL;
            if (node->childCount > 2) {
                condExp = node->children[2];
            }
            
            if (condExp != NULL && condExp->nodeType == NODE_EXP) {
                ASTNode* condOp = NULL;
                if (condExp->childCount > 1) {
                    condOp = condExp->children[1];
                }
                
                if (condOp != NULL && condOp->tokenType == RELOP) {
                    char* relopVal = condOp->attr.value;
                    RelOp relop;
                    if (strcmp(relopVal, "<") == 0) relop = RELOP_LT;
                    else if (strcmp(relopVal, ">") == 0) relop = RELOP_GT;
                    else if (strcmp(relopVal, "<=") == 0) relop = RELOP_LE;
                    else if (strcmp(relopVal, ">=") == 0) relop = RELOP_GE;
                    else if (strcmp(relopVal, "==") == 0) relop = RELOP_EQ;
                    else relop = RELOP_NE;
                    
                    char* leftVal = NULL;
                    char* rightVal = NULL;
                    if (condExp->childCount > 0) {
                        leftVal = genExpInternal(condExp->children[0], 1);
                    }
                    if (condExp->childCount > 2) {
                        rightVal = genExpInternal(condExp->children[2], 1);
                    }
                    
                    if (leftVal != NULL && rightVal != NULL) {
                        if (rightVal[0] == '#') {
                            char* tempVar = newTemp();
                            IRNode* assignIR = createIRNode(IR_ASSIGN);
                            assignIR->result = strdup(tempVar);
                            assignIR->arg2 = strdup(rightVal);
                            addIRNode(assignIR);
                            
                            IRNode* ifIR = createIRNode(IR_IF);
                            ifIR->arg1 = strdup(leftVal);
                            ifIR->arg2 = strdup(tempVar);
                            ifIR->relop = relop;
                            ifIR->result = strdup(labelThen);
                            addIRNode(ifIR);
                        } else {
                            IRNode* ifIR = createIRNode(IR_IF);
                            ifIR->arg1 = strdup(leftVal);
                            ifIR->arg2 = strdup(rightVal);
                            ifIR->relop = relop;
                            ifIR->result = strdup(labelThen);
                            addIRNode(ifIR);
                        }
                    }
                } else {
                    char* cond = genExpInternal(condExp, 1);
                    if (cond != NULL) {
                        IRNode* ifIR = createIRNode(IR_IF);
                        ifIR->arg1 = strdup(cond);
                        ifIR->arg2 = strdup("#0");
                        ifIR->relop = RELOP_NE;
                        ifIR->result = strdup(labelThen);
                        addIRNode(ifIR);
                    }
                }
            }
            
            IRNode* gotoIR1 = createIRNode(IR_GOTO);
            gotoIR1->result = strdup(labelElse);
            addIRNode(gotoIR1);
            
            IRNode* labelIR1 = createIRNode(IR_LABEL);
            labelIR1->result = strdup(labelThen);
            addIRNode(labelIR1);
            
            if (node->childCount > 4) {
                genStmt(node->children[4]);
            }
            
            IRNode* gotoIR2 = createIRNode(IR_GOTO);
            gotoIR2->result = strdup(labelEnd);
            addIRNode(gotoIR2);
            
            IRNode* labelIR2 = createIRNode(IR_LABEL);
            labelIR2->result = strdup(labelElse);
            addIRNode(labelIR2);
            
            if (node->childCount > 6) {
                genStmt(node->children[6]);
            }
            
            IRNode* labelIR3 = createIRNode(IR_LABEL);
            labelIR3->result = strdup(labelEnd);
            addIRNode(labelIR3);
        } else if (child->tokenType == WHILE) {
            char* labelLoop = newLabel();
            char* labelEnd = newLabel();
            
            IRNode* labelIR1 = createIRNode(IR_LABEL);
            labelIR1->result = strdup(labelLoop);
            addIRNode(labelIR1);
            
            ASTNode* condExp = NULL;
            if (node->childCount > 2) {
                condExp = node->children[2];
            }
            
            if (condExp != NULL && condExp->nodeType == NODE_EXP) {
                char* cond = genExp(condExp);
                if (cond != NULL) {
                    IRNode* ifIR = createIRNode(IR_IF);
                    ifIR->arg1 = strdup(cond);
                    ifIR->arg2 = strdup("#0");
                    ifIR->relop = RELOP_EQ;
                    ifIR->result = strdup(labelEnd);
                    addIRNode(ifIR);
                }
            }
            
            if (node->childCount > 4) {
                genStmt(node->children[4]);
            }
            
            IRNode* gotoIR = createIRNode(IR_GOTO);
            gotoIR->result = strdup(labelLoop);
            addIRNode(gotoIR);
            
            IRNode* labelIR2 = createIRNode(IR_LABEL);
            labelIR2->result = strdup(labelEnd);
            addIRNode(labelIR2);
        } else if (child->tokenType == RETURN) {
            char* val = NULL;
            if (node->childCount > 1) {
                val = genExp(node->children[1]);
            }
            if (val != NULL) {
                char* tempVar = newTemp();
                IRNode* assignIR = createIRNode(IR_ASSIGN);
                assignIR->result = strdup(tempVar);
                assignIR->arg2 = strdup(val);
                addIRNode(assignIR);
                
                IRNode* returnIR = createIRNode(IR_RETURN);
                returnIR->arg1 = strdup(tempVar);
                addIRNode(returnIR);
            }
        } else if (child->nodeType == NODE_EXP) {
            genExp(child);
        } else if (child->nodeType == NODE_COMPST) {
            genCompSt(child);
        } else if (child->nodeType == NODE_STMT) {
            genStmt(child);
        }
    } else if (node->nodeType == NODE_COMPST) {
        genCompSt(node);
    } else if (node->nodeType == NODE_EXP) {
        genExp(node);
    }
}

void genCompSt(ASTNode* node) {
    if (node == NULL) return;
    
    if (node->nodeType == NODE_COMPST) {
        ASTNode* defList = NULL;
        ASTNode* stmtList = NULL;
        
        for (int i = 0; i < node->childCount; i++) {
            ASTNode* child = node->children[i];
            if (child != NULL) {
                if (child->tokenType == LC) continue;
                if (child->tokenType == RC) continue;
                if (child->nodeType == NODE_DEFLIST) {
                    defList = child;
                } else if (child->nodeType == NODE_STMTLIST) {
                    stmtList = child;
                } else if (child->nodeType == NODE_STMT) {
                    genStmt(child);
                }
            }
        }
        
        if (defList != NULL) {
            genDefList(defList);
        }
        
        if (stmtList != NULL) {
            genStmtList(stmtList);
        }
    }
}

void genStmtList(ASTNode* node) {
    if (node == NULL) return;
    
    if (node->nodeType == NODE_STMTLIST) {
        for (int i = 0; i < node->childCount; i++) {
            ASTNode* child = node->children[i];
            if (child != NULL) {
                if (child->nodeType == NODE_STMT) {
                    genStmt(child);
                } else if (child->nodeType == NODE_STMTLIST) {
                    genStmtList(child);
                }
            }
        }
    }
}

void genDefList(ASTNode* node) {
    if (node == NULL) return;
    
    if (node->nodeType == NODE_DEFLIST) {
        for (int i = 0; i < node->childCount; i++) {
            ASTNode* child = node->children[i];
            if (child != NULL) {
                if (child->nodeType == NODE_DEF) {
                    genDef(child);
                } else if (child->nodeType == NODE_DEFLIST) {
                    genDefList(child);
                }
            }
        }
    }
}

void genDef(ASTNode* node) {
    if (node == NULL) return;
    
    if (node->nodeType == NODE_DEF) {
        ASTNode* decList = NULL;
        if (node->childCount > 1) {
            decList = node->children[1];
        }
        
        if (decList != NULL) {
            genDecList(decList);
        }
    }
}

void genDecList(ASTNode* node) {
    if (node == NULL) return;
    
    if (node->nodeType == NODE_DECLIST) {
        for (int i = 0; i < node->childCount; i++) {
            ASTNode* child = node->children[i];
            if (child != NULL && child->nodeType != NODE_TOKEN) {
                genDec(child);
            }
        }
    } else if (node->nodeType == NODE_EXTDECLIST) {
        for (int i = 0; i < node->childCount; i++) {
            ASTNode* child = node->children[i];
            if (child != NULL) {
                if (child->nodeType == NODE_VARDEC && child->childCount > 0) {
                    ASTNode* idNode = child->children[0];
                    if (idNode != NULL && idNode->tokenType == ID) {
                        if (child->childCount > 2) {
                            ASTNode* arrSize = child->children[2];
                            if (arrSize != NULL && arrSize->tokenType == INT) {
                                IRNode* decIR = createIRNode(IR_DEC);
                                decIR->result = strdup(getVarName(idNode->attr.value));
                                char* sizeStr = (char*)safeMalloc(10);
                                sprintf(sizeStr, "%d", arrSize->intValue * 4);
                                decIR->arg1 = sizeStr;
                                addIRNode(decIR);
                            }
                        }
                    }
                } else if (child->nodeType == NODE_EXTDECLIST) {
                    genDecList(child);
                }
            }
        }
    }
}

void genDec(ASTNode* node) {
    if (node == NULL) return;
    
    if (node->nodeType == NODE_DEC) {
        ASTNode* varDec = NULL;
        if (node->childCount > 0) {
            varDec = node->children[0];
        }
        
        if (varDec != NULL && varDec->nodeType == NODE_VARDEC && varDec->childCount > 0) {
            ASTNode* idNode = varDec->children[0];
            if (idNode != NULL && idNode->tokenType == ID) {
                if (varDec->childCount > 2) {
                    ASTNode* arrSize = varDec->children[2];
                    if (arrSize != NULL && arrSize->tokenType == INT) {
                        IRNode* decIR = createIRNode(IR_DEC);
                        decIR->result = strdup(idNode->attr.value);
                        char* sizeStr = (char*)safeMalloc(10);
                        sprintf(sizeStr, "%d", arrSize->intValue * 4);
                        decIR->arg1 = sizeStr;
                        addIRNode(decIR);
                    }
                }
                
                if (node->childCount > 1) {
                    ASTNode* assign = node->children[1];
                    if (assign != NULL && assign->tokenType == ASSIGNOP) {
                        ASTNode* exp = NULL;
                        if (node->childCount > 2) {
                            exp = node->children[2];
                        }
                        if (exp != NULL) {
                            char* rightVal = genExp(exp);
                            if (rightVal != NULL) {
                                IRNode* assignIR = createIRNode(IR_ASSIGN);
                                assignIR->result = strdup(idNode->attr.value);
                                assignIR->arg1 = strdup(rightVal);
                                addIRNode(assignIR);
                            }
                        }
                    }
                }
            }
        }
    }
}
