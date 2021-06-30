#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


//go to lex.h/////////////////////////////////////////////////////////////////////////////////////////
//LEX.H ==================================================================================================
#define MAXLEN 256

// Token types
typedef enum {
    UNKNOWN, 
    END, 
    ENDFILE, 
    INT, 
    ID,
    ADDSUB, 
    MULDIV,
    ASSIGN, 
    LPAREN, 
    RPAREN,
    INCDEC, 
    AND, OR, XOR
} TokenSet;

// Test if a token matches the current token 
extern int match(TokenSet token);

// Get the next token
extern void advance(void);

// Get the lexeme of the current token
extern char *getLexeme(void);


//go to parser.h/////////////////////////////////////////////////////////////////////////////////////////
//PARSER.H ===============================================================================================
#define TBLSIZE 64

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 1

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum) { \
    printf("EXIT 1\n");\
    if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
    err(errorNum); \
}

// Error types
typedef enum {
    UNDEFINED, MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NOTLVAL, DIVZERO, SYNTAXERR
} ErrorType;

// Structure of the symbol table
typedef struct {
    int val;
    char name[MAXLEN]; 
} Symbol;

// Structure of a tree node
typedef struct _Node {
    TokenSet data;
    int val;
    char lexeme[MAXLEN];
    struct _Node *left; 
    struct _Node *right;
} BTNode;

// The symbol table
extern Symbol table[TBLSIZE];

//debug The MOV idx update
extern int index_count_MOV;

// Initialize the symbol table with builtin variables
extern void initTable(void);


// Get the memory idx of a register 
extern int get_memory_register_idx(char *str, int is_assign); //debug nicole made function

// Get the value of a variable
extern int getval(char *str, int* id); 

// Set the value of a variable
extern int setval(char *str, int val, int* id);

// Make a new node according to token type and lexeme
extern BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
extern void freeTree(BTNode *root);

extern void statement(void);
extern BTNode *assign_expr(void);
extern BTNode *or_expr(void);
extern BTNode *or_expr_tail(BTNode *left);
extern BTNode *xor_expr(void);
extern BTNode *xor_expr_tail(BTNode *left);
extern BTNode *and_expr(void);
extern BTNode *and_expr_tail(BTNode *left);
extern BTNode *addsub_expr(void);
extern BTNode *addsub_expr_tail(BTNode *left);
extern BTNode *muldiv_expr(void);
extern BTNode *muldiv_expr_tail(BTNode *left);
extern BTNode *unary_expr(void);
extern BTNode *factor(void);


// Print error message and exit the program
extern void err(ErrorType errorNum);


//go to codegen.h/////////////////////////////////////////////////////////////////////////////////////////
//CODEGEN.H ==============================================================================================

int register_idx;
// int left_reg_idx, right_reg_idx;

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);
extern void printPostfix(BTNode *root);
extern void printInfix(BTNode *root);


//go to lex.c/////////////////////////////////////////////////////////////////////////////////////////
//LEX.C ==================================================================================================

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

TokenSet getToken(void)
{
    
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');

    if (isdigit(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN) {
            if(c=='_') error(SYNTAXERR);
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    } 
    else if (c == '+' || c == '-') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        
        char incdec_ma = fgetc(stdin);
        if(incdec_ma == lexeme[0]){
            lexeme[1] = incdec_ma;
            lexeme[2] = '\0';
            return INCDEC;
        }
        else ungetc(incdec_ma, stdin);
        
        return ADDSUB;
    } 
    else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } 
    else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    } 
    else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    } 
    else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    } 
    else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    } 
    else if (c == '&'){
        lexeme[0] = c;
        lexeme[1] = '\0';
        return AND;
    }
    else if (c == '|'){
        lexeme[0] = c;
        lexeme[1] = '\0';
        return OR;
    }
    else if(c == '^'){
        lexeme[0] = c;
        lexeme[1] = '\0';
        return XOR;
    }
    else if (isalpha(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while((isalpha(c) || c == '_'|| isdigit(c)) && i<MAXLEN){
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return ID;
    } 
    else if (c == EOF) {
        return ENDFILE;
    } 
    else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char *getLexeme(void) {
    return lexeme;
}

//go to parser.c/////////////////////////////////////////////////////////////////////////////////////////
//PARSER.C ===============================================================================================
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
            
            
            //printf("%d", evaluateTree(retp));
            for(int i=0;i<3;i++){
            printf("MOV r%d [%d]\n",i,i*4);
            }
            printf("EXIT 0\n");
            
            printf("\nPrefix traversal: ");
            printPrefix(retp);
            printf("\nInfix traversal: ");
            printInfix(retp);
            printf("\nPostfix traversal: ");
            printPostfix(retp);
            printf("\n");
            
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


//go to codegen.c/////////////////////////////////////////////////////////////////////////////////////////
//CODEGEN.C ==============================================================================================
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

//MAIN.C =================================================================================================  

int main() {
    initTable();
    //printf(">> ");
    
    while (1) {
        statement();
    }
    return 0;
}