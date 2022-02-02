%{
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <AST_Tree.h>
#include "scanType.h"  // TokenData Type
using namespace std;

// Declare stuff from Flex that Bison needs to know about:
extern int yylex();
extern int yyparse();
extern FILE *yyin;
extern int errNum;

extern void yyerror(const char *s);
%}

%union {
    ExpType type; // For passing types (i.e pass a type in a decl like int or bool)
    TokenData *tokenData; // For terminals. Token data comes from yylex() in the $ vars
    AST_Tree * tree; // For nonterminals. Add these nodes as you build the tree.
}

// define token types
%token <tokenData> BOOLCONST NUMCONST CHARCONST STRINGCONST ID
%token <tokenData> IF WHILE FOR STATIC INT BOOL CHAR ELSE RETURN BREAK
%token <tokenData> EQ ADDASS SUBASS DIVASS MULASS LEQ GEQ NEQ DEC INC
%token <tokenData> ADD SUB LT GT MUL DIV MOD RAND ASS AND OR NOT 
%token <tokenData> BEG END THEN ASGN TO BY DO

%start program

%type <tree> program decList declaration varDec scopedVarDec varDecList
%type <tree> varDecInit varDecId funDec params paramList call 65
%type <tree> paramTypeList paramIdList paramId statement closedStmt
%type <tree> openStmt expressionStmt compoundStmt localDecs stmtList
%type <tree> closedSelectStmt openSelectStmt closedIterationStmt
%type <tree> openIterationStmt iterationRange returnStmt breakStmt
%type <tree> expression assignop simpleExp andExp unaryRelExp relExp
%type <tree> sumExp mulExp unaryExp factor mutable immutable args argList 
%type <tree> constant typeSpec unaryop mulop sumop
%type <tokenData> relop
%%

//-------------------- Grammar Structure --------------------
program     
          : decList                       { printf("Correct Syntax\n"); }
          ;

decList   
          : decList declaration
          | declaration
          ;

declaration
          : varDec
          | funDec
          ;

//-------------------- Variables --------------------
varDec
          : typeSpec varDecList ';'
          ;

scopedVarDec
          : STATIC typeSpec varDecList ';'
          | typeSpec varDecList ';'
          ;

varDecList
          : varDecList ',' varDecInit
          | varDecInit
          ;

varDecInit
          : varDecId
          | varDecId ':' simpleExp
          ;

varDecId
          : ID 
          | ID '[' NUMCONST ']'
          ;

typeSpec
          : BOOL
          | CHAR
          | INT
          ;

//-------------------- Functions --------------------
funDec
          : typeSpec ID '(' params ')' compoundStmt
          |          ID '(' params ')' compoundStmt
          ;

params 
          : paramList
          |
          ;

paramList 
          : paramList ';' paramTypeList
          | paramTypeList
          ;

paramTypeList
          : typeSpec paramIdList
          ;

paramIdList
          : paramIdList ',' paramId
          | paramId
          ;

paramId
          : ID
          | ID '[' ']'
          ;

//-------------------- Statements --------------------
statement 
          : closedStmt
          | openStmt
          ;

closedStmt
          : expressionStmt
          | compoundStmt
          | closedSelectStmt
          | closedIterationStmt
          | returnStmt
          | breakStmt
          ;

openStmt
          : openSelectStmt
          | openIterationStmt
          ;

expressionStmt
          : expression ';'
          | ';'
          ;

compoundStmt
          : BEG localDecs stmtList END
          ;

localDecs
          : localDecs scopedVarDec
          |
          ;

stmtList 
          : stmtList statement
          |
          ;

// Dangling else
closedSelectStmt
          : IF simpleExp THEN closedStmt ELSE closedStmt
          ;

openSelectStmt
          : IF simpleExp THEN statement
          | IF simpleExp THEN closedStmt ELSE openStmt
          ;

closedIterationStmt
          : WHILE simpleExp DO closedStmt
          | FOR ID ASGN iterationRange DO closedStmt
          ;

openIterationStmt
          : WHILE simpleExp DO openStmt
          | FOR ID ASGN iterationRange DO openStmt
          ;

iterationRange
          : simpleExp TO simpleExp
          | simpleExp TO simpleExp BY simpleExp
          ;

returnStmt
          : RETURN ';'
          | RETURN expression ';'
          ;

breakStmt
          : BREAK ';'
          ;

//-------------------- Expressions --------------------
expression
          : mutable assignop expression
          | mutable INC
          | mutable DEC
          | simpleExp
          ;

assignop
          : ASGN
          | ADDASS
          | SUBASS
          | MULASS
          | DIVASS
          ;

simpleExp
          : simpleExp OR andExp
          | andExp
          ;

andExp
          : andExp AND unaryRelExp
          | unaryRelExp
          ;

unaryRelExp
          : NOT unaryRelExp
          | relExp
          ;

relExp
          : sumExp relop sumExp 
          | sumExp
          ;

relop
          : LT
          | GT
          | LEQ
          | GEQ
          | ASS
          | NEQ
          ;

sumExp
          : sumExp sumop mulExp
          | mulExp
          ;

sumop
          : ADD
          | SUB
          ;

mulExp
          : mulExp mulop unaryExp
          | unaryExp
          ;

mulop
          : MUL
          | DIV
          | MOD
          ;

unaryExp
          : unaryop unaryExp
          | factor
          ;

unaryop
          : SUB
          | MUL
          | RAND
          ;

factor
          : mutable
          | immutable
          ;

mutable
          : ID
          | ID '[' expression ']'
          ;

immutable
          : '(' expression ')'
          | call
          | constant
          ;

call
          : ID '(' args ')'
          ;

args
          : argList
          |
          ;

argList
          : argList ',' expression
          | expression
          ;

constant
          : NUMCONST
          | CHARCONST
          | STRINGCONST
          | BOOLCONST
          ;

%%

// C code to openfile
int main(int argc, char *argv[])
{
    if (argc > 1) {
        if ((yyin = fopen(argv[1], "r"))) {
            // file open successful
        }
        else {
            // failed to open file
            printf("ERROR: failed to open \'%s\'\n", argv[1]);
            exit(1);
        }
    }
    yyparse();
}

// needs to be updated?
void yyerror(const char *s) {
  printf("EEK, parse error on line: %d! Message: %s\n", 111, s);
  // might as well halt now:
  exit(-1);
}