%{
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
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
  TokenData *tokenData;
}

// define token types
%token <tokenData> BOOLCONST NUMCONST CHARCONST STRINGCONST ID
%token <tokenData> IF WHILE FOR STATIC INT BOOL CHAR ELSE RETURN BREAK
%token <tokenData> EQ ADDASS SUBASS DIVASS MULASS LEQ GEQ NEQ DEC INC
%token <tokenData> ADD SUB LT GT MUL DIV MOD RAND ASS AND OR NOT 
%token <tokenData> BEG END THEN LPAR RPAR LBRK RBRK SC CM CN
%token <tokenData> ASGN TO BY

%%

// Simple grammer to act as a driver

tokenlist  
            : tokenlist token               {}
            | token                         {}
            ;

token
            : ID                            { printf("Line %d Token: ID Value: %s\n", $1->line, $1->tokenString); }
            | CHARCONST                     { printf("Line %d Token: CHARCONST Value: \'%c\'  Input: %s\n", $1->line, $1->cValue, $1->tokenString); }
            | STRINGCONST                   { printf("Line %d Token: STRINGCONST Value: \"%s\"  Len: %lu  Input: %s\n", $1->line, $1->sValue.c_str(), ($1->sValue).size(), $1->tokenString); }
            | BOOLCONST                     { printf("Line %d Token: BOOLCONST Value: %d  Input: %s\n", $1->line, $1->nValue, $1->tokenString); }
            | NUMCONST                      { printf("Line %d Token: NUMCONST Value: %d  Input: %s\n", $1->line, $1->nValue, $1->tokenString);}
            | IF                            { printf("Line %d Token: %s\n", $1->line, "IF"); }
            | WHILE                         { printf("Line %d Token: %s\n", $1->line, "WHILE"); }
            | FOR                           { printf("Line %d Token: %s\n", $1->line, "FOR"); }
            | STATIC                        { printf("Line %d Token: %s\n", $1->line, "STATIC"); }
            | INT                           { printf("Line %d Token: %s\n", $1->line, "INT"); }
            | BOOL                          { printf("Line %d Token: %s\n", $1->line, "BOOL"); }
            | CHAR                          { printf("Line %d Token: %s\n", $1->line, "CHAR"); }
            | ELSE                          { printf("Line %d Token: %s\n", $1->line, "ELSE"); }
            | RETURN                        { printf("Line %d Token: %s\n", $1->line, "RETURN"); }
            | BREAK                         { printf("Line %d Token: %s\n", $1->line, "BREAK"); }
            | BEG                           { printf("Line %d Token: %s\n", $1->line, "BEGIN"); }
            | END                           { printf("Line %d Token: %s\n", $1->line, "END"); }
            | THEN                          { printf("Line %d Token: %s\n", $1->line, "THEN"); }
            | AND                           { printf("Line %d Token: %s\n", $1->line, "AND"); }
            | NOT                           { printf("Line %d Token: %s\n", $1->line, "NOT"); }
            | TO                            { printf("Line %d Token: %s\n", $1->line, "TO"); }
            | BY                            { printf("Line %d Token: %s\n", $1->line, "BY"); }
            | ASGN                          { printf("Line %d Token: %s\n", $1->line, "ASGN"); }
            | ADDASS                        { printf("Line %d Token: %s\n", $1->line, "ADDASGN"); }
            | SUBASS                        { printf("Line %d Token: %s\n", $1->line, "SUBSGN"); }
            | DIVASS                        { printf("Line %d Token: %s\n", $1->line, "DIVSGN"); }
            | MULASS                        { printf("Line %d Token: %s\n", $1->line, "MULSGN"); }
            | LEQ                           { printf("Line %d Token: %s\n", $1->line, "LEQ"); }
            | GEQ                           { printf("Line %d Token: %s\n", $1->line, "GEQ"); }
            | NEQ                           { printf("Line %d Token: %s\n", $1->line, "NEQ"); }
            | DEC                           { printf("Line %d Token: %s\n", $1->line, "DEC"); }
            | INC                           { printf("Line %d Token: %s\n", $1->line, "INC"); }
            | ADD                           { printf("Line %d Token: %s\n", $1->line, "+"); }
            | SUB                           { printf("Line %d Token: %s\n", $1->line, "-"); }
            | LT                            { printf("Line %d Token: %s\n", $1->line, "<"); }
            | GT                            { printf("Line %d Token: %s\n", $1->line, ">"); }
            | MUL                           { printf("Line %d Token: %s\n", $1->line, "*"); }
            | DIV                           { printf("Line %d Token: %s\n", $1->line, "/"); }
            | MOD                           { printf("Line %d Token: %s\n", $1->line, "%"); }
            | RAND                          { printf("Line %d Token: %s\n", $1->line, "?"); }
            | ASS                           { printf("Line %d Token: %s\n", $1->line, "="); }
            | OR                            { printf("Line %d Token: %s\n", $1->line, "OR"); }
            | LPAR                          { printf("Line %d Token: %s\n", $1->line, "("); }
            | RPAR                          { printf("Line %d Token: %s\n", $1->line, ")"); }
            | LBRK                          { printf("Line %d Token: %s\n", $1->line, "["); }
            | RBRK                          { printf("Line %d Token: %s\n", $1->line, "]"); }
            | SC                            { printf("Line %d Token: %s\n", $1->line, ";"); }
            | CM                            { printf("Line %d Token: %s\n", $1->line, ","); }
            | CN                            { printf("Line %d Token: %s\n", $1->line, ":"); }
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