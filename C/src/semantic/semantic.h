#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../parser/node.h"
#include "../parser/parser.tab.h"
#include "symtab.h"

void semanticAnalysis(ASTNode* root);

#endif