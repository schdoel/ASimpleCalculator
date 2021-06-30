#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"
#include "parser.h"
#include "lex.h"

int register_idx;
int flag_id;//debug

int evaluateTree(BTNode *root) {
    int id;
    int retval = 0, lv = 0, rv = 0; //cara e rh
    int left_reg_idx = register_idx;
    int right_reg_idx = register_idx+1;
    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme, &id);
                // printf("ID MOV r%d [%d]\n", register_idx, id);

                if(register_idx>7){
                    printf("MOV [%d] r%d\n", sbcount*4, register_idx%8);
                    sbcount++;
                    
                }

                printf("MOV r%d [%d]\n", register_idx%8, id);
                left_reg_idx = register_idx;
                register_idx++;
                right_reg_idx = register_idx;
                flag_id = 1; //debug for division
                break;
            case INT:
                retval = atoi(root->lexeme);
                // printf("INT MOV r%d %d\n", register_idx, retval);

                if(register_idx>7){
                    printf("MOV [%d] r%d\n", sbcount*4, register_idx%8);
                    sbcount++;
                }

                printf("MOV r%d %d\n", register_idx%8, retval);
                left_reg_idx = register_idx;
                register_idx++;
                right_reg_idx = register_idx;
                break;
            case ASSIGN: // coma eval right doang cok
                right_reg_idx = register_idx;
                rv = evaluateTree(root->right);
                retval = setval(root->left->lexeme, rv, &id);
                // printf("ASSIGN MOV [%d] r%d\n", id, right_reg_idx);
                printf("MOV [%d] r%d\n", id, right_reg_idx%8);
                left_reg_idx = register_idx;
                flag_id = 1;
                break;
            case ADDSUB:
            case MULDIV:
            case AND:
            case OR:
            case XOR:
                lv = evaluateTree(root->left);
                if (strcmp(root->lexeme, "+") == 0) {
                    rv = evaluateTree(root->right);//debug
                    retval = lv + rv;
                    printf("ADD r%d r%d\n", left_reg_idx%8, right_reg_idx%8);
                } 
                else if (strcmp(root->lexeme, "-") == 0) {
                    rv = evaluateTree(root->right);//debug
                    retval = lv - rv;
                    printf("SUB r%d r%d\n", left_reg_idx%8, right_reg_idx%8);
                } 
                else if (strcmp(root->lexeme, "*") == 0) {
                    rv = evaluateTree(root->right);//debug
                    retval = lv * rv;
                    printf("MUL r%d r%d\n", left_reg_idx%8, right_reg_idx%8);
                } 
                else if (strcmp(root->lexeme, "/") == 0) {
                    flag_id = 0;
                    rv = evaluateTree(root->right);//debug
                    // evaluateTree(root->right);
                    if (rv == 0 && flag_id==0){
                        error(DIVZERO);
                    }
                    if(rv == 0 && flag_id==1){//debug for division
                        retval = 0;
                    }
                    else retval = lv / rv;
                    printf("DIV r%d r%d\n", left_reg_idx%8, right_reg_idx%8);
                }
                else if (strcmp(root->lexeme, "&") == 0){
                    rv = evaluateTree(root->right);//debug
                    retval = lv & rv;
                    printf("AND r%d r%d\n", left_reg_idx%8, right_reg_idx%8);
                }
                else if (strcmp(root->lexeme, "|") == 0) {
                    rv = evaluateTree(root->right);//debug
                    retval = lv | rv;
                    printf("OR r%d r%d\n", left_reg_idx%8, right_reg_idx%8);
                }
                else if (strcmp(root->lexeme, "^") == 0) {
                    rv = evaluateTree(root->right);//debug
                    retval = lv ^ rv;
                    printf("XOR r%d r%d\n", left_reg_idx%8, right_reg_idx%8);
                }
                //x = 1+(2+(3+(4+(5+(6+(7+(8+(9+10))))))))
                right_reg_idx = register_idx;
                register_idx--;
                left_reg_idx = register_idx;
                if(register_idx>7){
                    sbcount--;
                    printf("MOV r%d [%d]\n", register_idx%8,sbcount*4);
                }
                break;
            case INCDEC:
            //process getval dulu dari root-> yang ID, terus evaluate tree setval, minta angka 1
                lv = evaluateTree(root->left); //ini si IDnya 
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "++") == 0) {
                    retval = setval(root->left->lexeme, lv+rv, &id);
                    printf("ADD r%d r%d\n", left_reg_idx%8, right_reg_idx%8);
                } 
                else if (strcmp(root->lexeme, "--") == 0) {
                    retval = setval(root->left->lexeme, lv-rv, &id);
                    printf("SUB r%d r%d\n", left_reg_idx%8, right_reg_idx%8);
                } 
                printf("MOV [%d] r%d\n", id, left_reg_idx%8); 
                right_reg_idx = register_idx;
                register_idx--;
                left_reg_idx = register_idx;
                if(register_idx>7){
                    sbcount--;
                    printf("MOV r%d [%d]\n", register_idx%8,sbcount*4);
                }
                break;
            
            default:
                retval = 0;
        }
    }
    return retval;
}

void printPrefix(BTNode *root) {
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }else{
        //printf("End ");
    }
}

void printPostfix(BTNode *root) {
    if (root != NULL) {
        printPostfix(root->left);
        printPostfix(root->right);
        printf("%s ", root->lexeme);
    }else{
        //printf("End ");
    }
}

void printInfix(BTNode *root) {
    if (root != NULL) {
        printInfix(root->left);
        printf("%s ",root->lexeme);
        printInfix(root->right);
    }else{
        //printf("End ");
    }
}