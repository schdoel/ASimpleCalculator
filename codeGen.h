#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"

int register_idx;
// int left_reg_idx, right_reg_idx;

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);
extern void printPostfix(BTNode *root);
extern void printInfix(BTNode *root);


#endif // __CODEGEN__
