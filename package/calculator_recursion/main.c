#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"
#include "parser.h"
#include "codeGen.h"

int main() {
    initTable();
    //printf(">> ");
    
    while (1) {
        statement();
    }
    return 0;
}



