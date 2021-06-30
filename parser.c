#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codeGen.h"

int index_count_MOV;
int sbcount = 0;
Symbol table[TBLSIZE];

void initTable(void) {
    strcpy(table[0].name, "x");
    table[0].val = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    sbcount = 3;
}

int getval(char *str, int *id) { //to get value of the nodes through recursion
    int i = 0;
    //printf("\ngetval %s", str); //debug
    for (i = 0; i < sbcount; i++){
        if (strcmp(str, table[i].name) == 0){
            *id = i*4;
            //printf("ID:%d\n", *id);
            //printf(": %d\n",table[i].val);
            return table[i].val;
        }
    }

    if (sbcount >= TBLSIZE){
        //printf("error runout getval\n"); // debug
        error(RUNOUT);
    }
    error(RUNOUT);
    return 0;
}

//ngirim lexeme left, dimasukin ke 
int setval(char *str, int val, int*id) {
    int i = 0;
    // printf("{%d}",val);
    // printf("<%s>\n", str);
    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            *id = i*4;
            return val;
        }
    }

    if (sbcount >= TBLSIZE){
        //printf("error more than tblsize\n"); //debug
        error(RUNOUT);
    }
    *id = sbcount*4;
    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}
//data type, the char
BTNode *makeNode(TokenSet tok, const char *lexe) {
    //printf(" NEW[%s]",lexe);
    BTNode *node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(BTNode *root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

BTNode *factor(void) {
    BTNode *retp = NULL;
    BTNode *left = NULL;

    //printf("YE");
    if (match(INT)) {
        // printf("INT ");
        retp = makeNode(INT, getLexeme());
        advance();
    } 
    else if (match(ID)) {
        // printf("ID ");
        retp = makeNode(ID, getLexeme());
        advance();//NYARI TOKEN BERIKUTNYA , lexemenya ganti, cur token ganti jadi berikutnya
    } 
    else if (match(INCDEC)) {
        retp = makeNode(INCDEC, getLexeme());
        retp->right = makeNode(INT, "1");
        advance();
    
        if (match(ID)) {
            retp->left = makeNode(ID, getLexeme());
            advance();
        } 
        else {
            // printf("error factor, incdec\n"); // debug
            error(SYNTAXERR);
        }
    } 

    else if (match(LPAREN)) {
        advance();
       // printf("ceka \n");
        retp = assign_expr();
        //printf("\n {%s} \n",getLexeme());
        //printf("cekb \n");
        if (match(RPAREN)){
            advance();
        }
        else{
            // printf("error factor, lparen\n"); //debug
            error(MISPAREN);
        }
    } 
    else {
        // printf("error factor, else\n"); //debug
        error(NOTNUMID);
    }
    // printf("{return %s ",retp->lexeme);
    // printf("%s}\n",getLexeme());
    return retp;
}

// expr := term expr_tail
BTNode *assign_expr(void) {
    // printf(" assign"); //debug
    BTNode *node = NULL;
    BTNode* left = or_expr();

    if (match(ASSIGN)){
        if(left->data == ID){
            node = makeNode(ASSIGN, getLexeme());
            advance();
            node->left = left;
            node->right = assign_expr(); 
            return node;
        }
        else {
            // printf("error assign_expr\n"); //debug
            error(NOTNUMID);
            return NULL;
        }
    }
    else return left;
}

BTNode *unary_expr(void) {

    BTNode *node = NULL;
    // printf(" unary");

    if (match(ADDSUB)) {
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = makeNode(INT, "0");
        node->right = unary_expr();
        return node;
    }
    else {
        return factor();
    }
}

BTNode *or_expr(void){
    BTNode * node = xor_expr();
    return or_expr_tail(node);
}

BTNode *or_expr_tail(BTNode *left) {

    BTNode *node = NULL;
    // printf(" or");

    if (match(OR)) {
        node = makeNode(OR, getLexeme());
        advance();
        node->left = left;
        node->right = xor_expr();
        return or_expr_tail(node);
    }
    else {
        return left;
    }
}

BTNode *xor_expr(void){
    BTNode * node = and_expr();
    return xor_expr_tail(node);
}

BTNode *xor_expr_tail(BTNode *left) {

    BTNode *node = NULL;
    // printf(" xor");
    if (match(XOR)) {
        node = makeNode(XOR, getLexeme());
        advance();
        node->left = left;
        node->right = and_expr();
        return xor_expr_tail(node);
    }
    else {
        return left;
    }
}

BTNode *and_expr(void){
    BTNode * node = addsub_expr();
    return and_expr_tail(node);
}

BTNode *and_expr_tail(BTNode *left) {
    
    BTNode *node = NULL;
    // printf(" and");
    if (match(AND)) {
        node = makeNode(AND, getLexeme());
        advance();
        node->left = left;
        node->right = addsub_expr();
        return and_expr_tail(node);
    }
    else {
        return left;
    }
}

BTNode *addsub_expr(void){
    BTNode * node = muldiv_expr();
    return addsub_expr_tail(node);
}

BTNode *addsub_expr_tail(BTNode *left) {

    BTNode *node = NULL;
    // printf(" addsub");

    if (match(ADDSUB)) {
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = left;
        node->right = muldiv_expr();
        return addsub_expr_tail(node);
    }
    else {
        return left;
    }
}

BTNode *muldiv_expr(void){
    BTNode * node = unary_expr();
    return muldiv_expr_tail(node);
}

BTNode *muldiv_expr_tail(BTNode *left) {

    BTNode *node = NULL;
    // printf(" muldiv");

    if (match(MULDIV)) {
        node = makeNode(MULDIV, getLexeme());
        advance();
        node->left = left;
        node->right = unary_expr();
        return muldiv_expr_tail(node);
    }
    else {
        return left;
    }
}

// statement := ENDFILE | END | expr END
void statement(void) {
    BTNode *retp = NULL;
    //printf(" statement");

    if (match(ENDFILE)) {
        for(int i=0;i<3;i++){
            printf("MOV r%d [%d]\n",i,i*4);
        }
        printf("EXIT 0\n");
        exit(0);
    } else if (match(END)) {
        // printf(">> ");
        advance();
    } else {
        retp = assign_expr();
       
        if (match(END)) {
            // left_reg_idx = right_reg_idx = 
            register_idx = 0;
            int x = evaluateTree(retp); //recomment to submit
            
            /*
            printf("%d", evaluateTree(retp));
            printf("\nPrefix traversal: ");
            printPrefix(retp);
            printf("\nInfix traversal: ");
            printInfix(retp);
            printf("\nPostfix traversal: ");
            printPostfix(retp);
            printf("\n");
            */
            // //debug
            // // printf("\n");

            // for(int i=0;i<11;i++){
            //     printf("%s:",table[i].name);
            //     printf("%d ",table[i].val);
            // }
            // printf("\n");
            

            freeTree(retp);
            // for(int i=0;i<3;i++){
            //     printf("r[%d] = %d\n",i,table[i].val);
            // }
            //printf(">> ");
            advance();
        } else {
            // printf("error statement, else\n"); //debug
            error(SYNTAXERR);
        }
    }
}

void err(ErrorType errorNum) {
    if (PRINTERR) {
        fprintf(stderr, "error: ");
        switch (errorNum) {
            case MISPAREN:
                fprintf(stderr, "mismatched parenthesis\n");
                break;
            case NOTNUMID:
                fprintf(stderr, "number or identifier expected\n");
                break;
            case NOTFOUND:
                fprintf(stderr, "variable not defined\n");
                break;
            case RUNOUT:
                fprintf(stderr, "out of memory\n");
                break;
            case NOTLVAL:
                fprintf(stderr, "lvalue required as an operand\n");
                break;
            case DIVZERO:
                fprintf(stderr, "divide by constant zero\n");
                break;
            case SYNTAXERR:
                fprintf(stderr, "syntax error\n");
                break;
            default:
                fprintf(stderr, "undefined error\n");
                break;
        }
    }
    exit(0);
}

