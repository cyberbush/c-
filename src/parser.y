%{
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "scanType.h"       // TokenData Type
#include "AST_Tree.h"       // AST Tree
#include "utils.cpp"          // Utility functions
#include "printTree.cpp"      // Printing AST tree
using namespace std;

// Declare stuff from Flex that Bison needs to know about:
extern int yylex();
extern int yyparse();
extern FILE *yyin;
extern int errNum;

static AST_Tree* root; // root of the tree

extern void yyerror(const char *s);
%}

%union {
    // ExpType type; // For passing types (i.e pass a type in a decl like int or bool)
    struct TokenData *tokenData; // For terminals. Token data comes from yylex() in the $ vars
    struct AST_Tree* tree; // For nonterminals. Add these nodes as you build the tree.
}

// define token types
%token <tokenData> BOOLCONST NUMCONST CHARCONST STRINGCONST ID
%token <tokenData> IF WHILE FOR STATIC INT BOOL CHAR ELSE RETURN BREAK
%token <tokenData> EQ ADDASS SUBASS DIVASS MULASS LEQ GEQ NEQ DEC INC
%token <tokenData> ADD SUB LT GT MUL DIV MOD RAND ASS AND OR NOT 
%token <tokenData> BEG END THEN ASGN TO BY DO

%start program

%type <tree> program decList declaration varDec scopedVarDec varDecList
%type <tree> varDecInit varDecId funDec params paramList call
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
          : decList                         { root = $1; /* save tree */ }
          ;

decList   
          : decList declaration             { 
                                                AST_Tree* t = $1; 
                                                if(t != NULL) {
                                                    while(t->sibling != NULL){ // keep traversing right
                                                        t = t->sibling;
                                                    }
                                                t->sibling = $2;
                                                $$ = $1;
                                                } else $$ = $2;
                                            }
          | declaration                     { $$ = $1; }
          ;

declaration
          : varDec                          { $$ = $1; }
          | funDec                          { $$ = $1; }
          ;

//-------------------- Variables --------------------
varDec
          : typeSpec varDecList ';'         {
                                                $$ = $2;
                                                AST_Tree* t = $2;
                                                while(t != NULL) { // go through and save the expType
                                                    t->expType = $1->expType;
                                                    if(t->child[0] != NULL && t->child[0]->subkind.exp == InitK) t->child[0]->expType = $1->expType;
                                                    t = t->sibling;
                                                }
                                            }
          ;

scopedVarDec
          : STATIC typeSpec varDecList ';'  {
                                                $$ = $3;
                                                AST_Tree* t = $3;
                                                while(t != NULL) {
                                                    t->expType = $2->expType;
                                                    if(t->child[0] != NULL && t->child[0]->subkind.exp == InitK) t->child[0]->expType = $2->expType;
                                                    t->isStatic = true;
                                                    t->varKind = LocalStatic;
                                                    t = t->sibling;
                                                } 
                                            }
          | typeSpec varDecList ';'         {
                                                $$ = $2;
                                                AST_Tree* t = $2;
                                                while(t != NULL) {
                                                    t->expType = $1->expType;
                                                    if(t->child[0] != NULL && t->child[0]->subkind.exp == InitK) t->child[0]->expType = $1->expType;
                                                    t = t->sibling;
                                                }
                                            }
          ;

varDecList
          : varDecList ',' varDecInit       {
                                                if($1 != NULL) {
                                                    AST_Tree* t = $1;
                                                    while(t->sibling != NULL)
                                                        t = t->sibling;
                                                    t->sibling = $3;
                                                    $$ = $1;
                                                }
                                                else $$ = $3;
                                            }
          | varDecInit                      { $$ = $1; }
          ;

varDecInit
          : varDecId                        { $$ = $1; }
          | varDecId ':' simpleExp          {
                                                $$ = $1;
                                                if($3 != NULL){
                                                    $$->child[0] = $3;
                                                    $$->subkind.exp = InitK;
                                                }
                                            }
          ;

varDecId
          : ID                              {
                                                $$ = createDeclNode(VarK, UndefinedType, $1->tokenString, $1->line, NULL, NULL, NULL);
                                            }
          | ID '[' NUMCONST ']'             {
                                                $$ = createDeclNode(VarK, UndefinedType, $1->tokenString, $1->line, NULL, NULL, NULL);
                                                $$->isArray = true;
                                            }
          ;

typeSpec
          : BOOL                            {
                                                $$ = createNodeFromToken($1, 0);
                                            }
          | CHAR                            {
                                                $$ = createNodeFromToken($1, 1);
                                            }
          | INT                             {
                                                $$ = createNodeFromToken($1, 2);
                                            }
          ;

//-------------------- Functions --------------------
funDec
          : typeSpec ID '(' params ')' compoundStmt         {
                                                                $$ = createDeclNode(FuncK, $1->expType, $2->tokenString, $2->line, $4, $6, NULL);
                                                                $6->attrib.name = $2->tokenString; // save name for scope 
                                                            }
          |          ID '(' params ')' compoundStmt         {
                                                                $$ = createDeclNode(FuncK, Void, $1->tokenString, $1->line, $3, $5, NULL);
                                                                $5->attrib.name = $1->tokenString;  // save name for scope                                                 
                                                            }
          ;

params 
          : paramList                       { $$ = $1; }
          | %empty                          { $$ = NULL; }
          ;

paramList 
          : paramList ';' paramTypeList     {
                                                $$ = $1;
                                                if($1 != NULL) {
                                                    AST_Tree* t = $1;
                                                    while(t->sibling != NULL) {
                                                        t = t->sibling;
                                                    }
                                                    t->sibling = $3;
                                                }
                                            }
          | paramTypeList                   { $$ = $1; }
          ;

paramTypeList
          : typeSpec paramIdList            {
                                                $$ = $2;
                                                AST_Tree* t = $2;
                                                while(t != NULL) {
                                                    t->expType = $1->expType;
                                                    t = t->sibling;
                                                }
                                            }
          ;

paramIdList
          : paramIdList ',' paramId         {
                                                $$ = $1;
                                                if($1 != NULL) {
                                                    AST_Tree* t = $1;
                                                    while(t->sibling != NULL) {
                                                        t = t->sibling;
                                                    }
                                                    t->sibling = $3;
                                                }
                                            }
          | paramId                         { $$ = $1; }
          ;

paramId
          : ID                              {
                                                $$ = createDeclNode(ParamK, UndefinedType, $1->tokenString, $1->line, NULL, NULL, NULL);
                                            }
          | ID '[' ']'                      {
                                                $$ = createDeclNode(ParamK, UndefinedType, $1->tokenString, $1->line, NULL, NULL, NULL);
                                                $$->isArray = true;
                                            }
          ;

//-------------------- Statements --------------------
statement 
          : closedStmt                      { $$ = $1; }
          | openStmt                        { $$ = $1; }
          ;

closedStmt
          : expressionStmt                  { $$ = $1; }
          | compoundStmt                    { $$ = $1; }
          | returnStmt                      { $$ = $1; }
          | breakStmt                       { $$ = $1; }
          | closedSelectStmt                { $$ = $1; }
          | closedIterationStmt             { $$ = $1; }
          ;

openStmt
          : openSelectStmt                  { $$ = $1; }
          | openIterationStmt               { $$ = $1; }
          ;

expressionStmt
          : expression ';'                  { $$ = $1; }
          | ';'                             { $$ = NULL; }
          ;

compoundStmt
          : BEG localDecs stmtList END      {
                                                $$ = createStmtNode(CompoundK, "", $1->line, $2, $3, NULL);
                                                $$->attrib.name = strdup("comp scope");
                                            }
          ;

localDecs
          : localDecs scopedVarDec          {
                                                AST_Tree* t = $1;
                                                if(t != NULL)
                                                {
                                                    while(t->sibling != NULL) {
                                                        t = t->sibling;
                                                    }
                                                    t->sibling = $2;
                                                    $$ = $1;
                                                } else $$ = $2;
                                            }
          | %empty                          { $$ = NULL; }
          ;

stmtList 
          : stmtList statement              {
                                                AST_Tree* t = $1;
                                                if(t != NULL) {
                                                    while(t->sibling != NULL) {
                                                        t = t->sibling;
                                                    }
                                                    t->sibling = $2;
                                                    $$ = $1;
                                                } else $$ = $2;
                                            }
          | %empty                          { $$ = NULL; }
          ;

// Dangling else
closedSelectStmt
          : IF simpleExp THEN closedStmt ELSE closedStmt        { $$ = createStmtNode(IfK, "if", $1->line, $2, $4, $6); }
          ;

openSelectStmt
          : IF simpleExp THEN statement                         { $$ = createStmtNode(IfK, "if", $1->line, $2, $4, NULL); }
          | IF simpleExp THEN closedStmt ELSE openStmt          { $$ = createStmtNode(IfK, "if", $1->line, $2, $4, $6); }
          ;

closedIterationStmt
          : WHILE simpleExp DO closedStmt                       { $$ = createStmtNode(WhileK, "", $1->line, $2, $4, NULL); }
          | FOR ID ASGN iterationRange DO closedStmt            {
                                                                    AST_Tree* n = createNodeFromToken($2, -1);
                                                                    n->nodeKind = DeclK;
                                                                    n->subkind.decl = VarK;
                                                                    n->expType = Integer;
                                                                    n->isInitialized = true;
                                                                    $$ = createStmtNode(ForK, "", $1->line, n, $4, $6);
                                                                }
          ;

openIterationStmt
          : WHILE simpleExp DO openStmt                         { $$ = createStmtNode(WhileK, "", $1->line, $2, $4, NULL); }
          | FOR ID ASGN iterationRange DO openStmt              {
                                                                    AST_Tree* n = createNodeFromToken($2, -1);
                                                                    n->nodeKind = DeclK;
                                                                    n->subkind.decl = VarK;
                                                                    n->expType = Integer;
                                                                    n->isInitialized = true;
                                                                      $$ = createStmtNode(ForK, "", $1->line, n, $4, $6);
                                                                }
          ;

iterationRange
          : simpleExp TO simpleExp                      { $$ = createStmtNode(RangeK, "", $2->line, $1, $3, NULL); }
          | simpleExp TO simpleExp BY simpleExp         { $$ = createStmtNode(RangeK, "", $2->line, $1, $3, $5); }
          ;

returnStmt
          : RETURN ';'                                  {
                                                            $$ = createStmtNode(ReturnK, "", $1->line, NULL, NULL, NULL);
                                                            $$->expType = Integer; // the type associated with the default return value
                                                            $$->attrib.value = 0; // the value returned by non-specific return statements
                                                        }
          | RETURN expression ';'                       {
                                                            $$ = createStmtNode(ReturnK, "", $1->line, $2, NULL, NULL);
                                                            $$->expType = UndefinedType;
                                                        }
          ;

breakStmt
          : BREAK ';'                                   { $$ = createStmtNode(BreakK, "", $1->line, NULL, NULL, NULL); }
          ;

//-------------------- Expressions --------------------
expression
          : mutable assignop expression                 { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
          | mutable INC                                 { 
                                                            $$ = createNodeFromToken($2, 3);
                                                            $$->child[0] = $1;
                                                        }
          | mutable DEC                                 {
                                                            $$ = createNodeFromToken($2, 3);
                                                            $$->child[0] = $1;                                                        
                                                        }
          | simpleExp                                   { $$ = $1; }
          ;

assignop
          : ASGN                                        { $$ = createNodeFromToken($1, 3); }
          | ADDASS                                      { $$ = createNodeFromToken($1, 3); }
          | SUBASS                                      { $$ = createNodeFromToken($1, 3); }
          | MULASS                                      { $$ = createNodeFromToken($1, 3); }
          | DIVASS                                      { $$ = createNodeFromToken($1, 3); }
          ;

simpleExp
          : simpleExp OR andExp                         { $$ = createOpNode($2->tokenString, $2->line, $1, $3, NULL); }
          | andExp                                      { $$ = $1; }
          ;

andExp
          : andExp AND unaryRelExp                      { $$ = createOpNode($2->tokenString, $2->line, $1, $3, NULL); }
          | unaryRelExp                                 { $$ = $1; }
          ;

unaryRelExp
          : NOT unaryRelExp                             { $$ = createOpNode($1->tokenString, $1->line, $2, NULL, NULL); }
          | relExp                                      { $$ = $1; }
          ;

relExp
          : sumExp relop sumExp                         { $$ = createOpNode($2->tokenString, $2->line, $1, $3, NULL); }
          | sumExp                                      { $$ = $1; }
          ;

relop
          : LT                                  { $$ = $1; }
          | GT                                  { $$ = $1; }
          | LEQ                                 { $$ = $1; }
          | GEQ                                 { $$ = $1; }
          | ASS                                 { $$ = $1; }
          | NEQ                                 { $$ = $1; }
          ;

sumExp
          : sumExp sumop mulExp                 { $$ = createOpNode($2->attrib.name, $2->lineNum, $1, $3, NULL); }
          | mulExp                              { $$ = $1; }
          ;

sumop
          : ADD                                 { $$ = createNodeFromToken($1, 4); }
          | SUB                                 { $$ = createNodeFromToken($1, 4); }
          ;

mulExp
          : mulExp mulop unaryExp               
          | unaryExp
          ;

mulop
          : MUL                                 { $$ = createNodeFromToken($1, 4); }
          | DIV                                 { $$ = createNodeFromToken($1, 4); }
          | MOD                                 { $$ = createNodeFromToken($1, 4); }
          ;

unaryExp
          : unaryop unaryExp
          | factor
          ;

unaryop
          : SUB                                 { $$ = createOpNode("chsign", $1->line, NULL, NULL, NULL); }
          | MUL                                 { $$ = createOpNode("sizeof", $1->line, NULL, NULL, NULL); }
          | RAND                                { $$ = createOpNode($1->tokenString, $1->line, NULL, NULL, NULL); }
          ;

factor
          : mutable                             { $$ = $1; }
          | immutable                           { $$ = $1; }
          ;

mutable
          : ID                                  { $$ = createNodeFromToken($1, 5); }
          | ID '[' expression ']'               { $$ = createOpNode("[", $1->line, createNodeFromToken($1, 5), $3, NULL); }
          ;

immutable
          : '(' expression ')'                  { $$ = $2; }
          | call                                { $$ = $1; }
          | constant                            { $$ = $1; }
          ;

call
          : ID '(' args ')'                     {
                                                    $$ = createNode(ExpK);
                                                    $$->subkind.exp = CallK;
                                                    $$->attrib.name = strdup($1->tokenString);
                                                    $$->lineNum = $1->line;
                                                    $$->num_params = countSiblings($3);   // save number of params
                                                    if($3 != NULL) $$->child[0] = $3;
                                                }     
          ;

args
          : argList                             { $$ = $1; }
          | %empty                              { $$ = NULL; }
          ;

argList
          : argList ',' expression              {
                                                    AST_Tree* t = $1;
                                                    if (t != NULL) {
                                                        while(t->sibling != NULL) {
                                                            t = t->sibling;
                                                        }
                                                        t->sibling = $3;
                                                        $$ = $1;
                                                    } else $$ = $3;
                                                }
          | expression                          { $$ = $1; }
          ;

constant
          : NUMCONST                            {
                                                    $$ = createNodeFromToken($1, 6);
                                                    $$->expType = Integer;
                                                }
          | CHARCONST                           {
                                                    $$ = createNodeFromToken($1, 6);
                                                    $$->expType = Char;
                                                }
          | STRINGCONST                         {
                                                    $$ = createNodeFromToken($1, 6);
                                                    $$->expType = Char;
                                                    $$->varKind = Global;
                                                    $$->isArray = true;
                                                }
          | BOOLCONST                           {   
                                                    $$ = createNodeFromToken($1, 6);
                                                    $$->expType = Boolean;
                                                }
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
    printAST(root, -1, 0);
}

// needs to be updated?
void yyerror(const char *s) {
  printf("EEK, parse error on line: %d! Message: %s\n", 111, s);
  // might as well halt now:
  exit(-1);
}