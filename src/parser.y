%{
#include "main.cpp"
using namespace std;

AST_Node* root;      // root of the tree

extern void yyerror(const char *s);
%}

%union {
    // ExpType type; // For passing types (i.e pass a type in a decl like int or bool)
    struct TokenData *tokenData; // For terminals. Token data comes from yylex() in the $ vars
    struct AST_Node* tree; // For nonterminals. Add these nodes as you build the tree.
}

// define token types
%token <tokenData> BOOLCONST NUMCONST CHARCONST STRINGCONST ID
%token <tokenData> IF WHILE FOR STATIC INT BOOL CHAR ELSE RETURN BREAK
%token <tokenData> ADDASS SUBASS DIVASS MULASS LEQ GEQ NEQ DEC INC
%token <tokenData> ADD SUB LT GT MUL DIV MOD RAND EQ AND OR NOT 
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
                                                AST_Node* t = $1; 
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
                                                AST_Node* t = $2;
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
                                                AST_Node* t = $3;
                                                while(t != NULL) {
                                                    t->expType = $2->expType;
                                                    if(t->child[0] != NULL && t->child[0]->subkind.exp == InitK) t->child[0]->expType = $2->expType;
                                                    t->isStatic = true;
                                                    t->varKind = LocalStatic;
                                                    t = t->sibling;
                                                } 
                                                removeToken(&$1);
                                            }
          | typeSpec varDecList ';'         {
                                                $$ = $2;
                                                AST_Node* t = $2;
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
                                                    AST_Node* t = $1;
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
                                                    $$->isInitialized = true;
                                                    //$$->isDeclUsed = true;
                                                    $$->hasInit = true;
                                                }
                                            }
          ;

varDecId
          : ID                              {
                                                $$ = createDeclNode(VarK, UndefinedType, $1->tokenString, $1->line, NULL, NULL, NULL);
                                                $$->size = 1;
                                                removeToken(&$1);
                                            }
          | ID '[' NUMCONST ']'             {
                                                $$ = createDeclNode(VarK, UndefinedType, $1->tokenString, $1->line, NULL, NULL, NULL);
                                                $$->isArray = true;
                                                $$->size = $3->nValue+1;
                                                removeToken(&$1);
                                                removeToken(&$3);
                                            }
          ;

typeSpec
          : BOOL                            {
                                                $$ = createNodeFromToken($1, 0);
                                                removeToken(&$1);
                                            }
          | CHAR                            {
                                                $$ = createNodeFromToken($1, 1);
                                                removeToken(&$1);
                                            }
          | INT                             {
                                                $$ = createNodeFromToken($1, 2);
                                                removeToken(&$1);
                                            }
          ;

//-------------------- Functions --------------------
funDec
          : typeSpec ID '(' params ')' compoundStmt         {
                                                                $$ = createDeclNode(FuncK, $1->expType, $2->tokenString, $2->line, $4, $6, NULL);
                                                                $6->name = $2->tokenString; // save name for scope 
                                                                removeToken(&$2);
                                                            }
          |          ID '(' params ')' compoundStmt         {
                                                                $$ = createDeclNode(FuncK, Void, $1->tokenString, $1->line, $3, $5, NULL);
                                                                $5->name = $1->tokenString;  // save name for scope     
                                                                removeToken(&$1);                                            
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
                                                    AST_Node* t = $1;
                                                    while(t->sibling != NULL) {
                                                        t = t->sibling;
                                                        if(t->isArray) t->size = 1;
                                                    }
                                                    t->sibling = $3;
                                                }
                                            }
          | paramTypeList                   { $$ = $1; }
          ;

paramTypeList
          : typeSpec paramIdList            {
                                                $$ = $2;
                                                AST_Node* t = $2;
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
                                                    AST_Node* t = $1;
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
                                                removeToken(&$1);
                                            }
          | ID '[' ']'                      {
                                                $$ = createDeclNode(ParamK, UndefinedType, $1->tokenString, $1->line, NULL, NULL, NULL);
                                                $$->isArray = true;
                                                removeToken(&$1);
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
                                                $$->name = strdup("comp scope");
                                                removeToken(&$1);
                                                removeToken(&$4);
                                            }
          ;

localDecs
          : localDecs scopedVarDec          {
                                                AST_Node* t = $1;
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
                                                AST_Node* t = $1;
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
          : IF simpleExp THEN closedStmt ELSE closedStmt        { 
                                                                    $$ = createStmtNode(IfK, "if", $1->line, $2, $4, $6); 
                                                                    removeToken(&$1);
                                                                    removeToken(&$3);
                                                                    removeToken(&$5);
                                                                }
          ;

openSelectStmt
          : IF simpleExp THEN statement                         { 
                                                                    $$ = createStmtNode(IfK, "if", $1->line, $2, $4, NULL); 
                                                                    removeToken(&$1);
                                                                    removeToken(&$3);
                                                                }
          | IF simpleExp THEN closedStmt ELSE openStmt          { 
                                                                    $$ = createStmtNode(IfK, "if", $1->line, $2, $4, $6); 
                                                                    removeToken(&$1);
                                                                    removeToken(&$3);
                                                                    removeToken(&$5);
                                                                }
          ;

closedIterationStmt
          : WHILE simpleExp DO closedStmt                       { 
                                                                    $$ = createStmtNode(WhileK, "", $1->line, $2, $4, NULL); 
                                                                    removeToken(&$1);
                                                                    removeToken(&$3);
                                                                }
          | FOR ID ASGN iterationRange DO closedStmt            {
                                                                    AST_Node* n = createNodeFromToken($2, -1);
                                                                    n->nodeKind = DeclK;
                                                                    n->subkind.decl = VarK;
                                                                    n->expType = Integer;
                                                                    n->isInitialized = true;
                                                                    $$ = createStmtNode(ForK, "", $1->line, n, $4, $6);
                                                                    removeToken(&$1);
                                                                    removeToken(&$2);
                                                                    removeToken(&$3);
                                                                    removeToken(&$5);
                                                                }
          ;

openIterationStmt
          : WHILE simpleExp DO openStmt                         { 
                                                                    $$ = createStmtNode(WhileK, "", $1->line, $2, $4, NULL); 
                                                                    removeToken(&$1);
                                                                    removeToken(&$3);
                                                                }
          | FOR ID ASGN iterationRange DO openStmt              {
                                                                    AST_Node* n = createNodeFromToken($2, -1);
                                                                    n->nodeKind = DeclK;
                                                                    n->subkind.decl = VarK;
                                                                    n->expType = Integer;
                                                                    n->isInitialized = true;
                                                                    n->size = 1;
                                                                    $$ = createStmtNode(ForK, "", $1->line, n, $4, $6);
                                                                    removeToken(&$1);
                                                                    removeToken(&$2);
                                                                    removeToken(&$3);
                                                                    removeToken(&$5);
                                                                }
          ;

iterationRange
          : simpleExp TO simpleExp                      { 
                                                            $$ = createStmtNode(RangeK, "", $2->line, $1, $3, NULL);
                                                            removeToken(&$2);
                                                        }
          | simpleExp TO simpleExp BY simpleExp         { 
                                                            $$ = createStmtNode(RangeK, "", $2->line, $1, $3, $5); 
                                                            removeToken(&$2);
                                                            removeToken(&$4);
                                                        }
          ;

returnStmt
          : RETURN ';'                                  {
                                                            $$ = createStmtNode(ReturnK, "", $1->line, NULL, NULL, NULL);
                                                            $$->expType = Integer; // the type associated with the default return value
                                                            $$->attrib.value = 0; // the value returned by non-specific return statements
                                                            removeToken(&$1);
                                                        }
          | RETURN expression ';'                       {
                                                            $$ = createStmtNode(ReturnK, "", $1->line, $2, NULL, NULL);
                                                            $$->expType = UndefinedType;
                                                            removeToken(&$1);
                                                        }
          ;

breakStmt
          : BREAK ';'                                   { 
                                                            $$ = createStmtNode(BreakK, "", $1->line, NULL, NULL, NULL); 
                                                            removeToken(&$1);
                                                        }
          ;

//-------------------- Expressions --------------------
expression
          : mutable assignop expression                 { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
          | mutable INC                                 { 
                                                            $$ = createNodeFromToken($2, 3);
                                                            $$->child[0] = $1;
                                                            removeToken(&$2);
                                                        }
          | mutable DEC                                 {
                                                            $$ = createNodeFromToken($2, 3);
                                                            $$->child[0] = $1;  
                                                            removeToken(&$2);                                                      
                                                        }
          | simpleExp                                   { $$ = $1; }
          ;

assignop
          : ASGN                                        { 
                                                            $$ = createNodeFromToken($1, 3); 
                                                            removeToken(&$1);
                                                        }
          | ADDASS                                      { 
                                                            $$ = createNodeFromToken($1, 3); 
                                                            removeToken(&$1);
                                                        }
          | SUBASS                                      { 
                                                            $$ = createNodeFromToken($1, 3); 
                                                            removeToken(&$1);
                                                        }
          | MULASS                                      { 
                                                            $$ = createNodeFromToken($1, 3); 
                                                            removeToken(&$1);
                                                        }
          | DIVASS                                      { 
                                                            $$ = createNodeFromToken($1, 3); 
                                                            removeToken(&$1);
                                                        }
          ;

simpleExp
          : simpleExp OR andExp                         { 
                                                            $$ = createOpNode($2->tokenString, $2->line, $1, $3, NULL); 
                                                            removeToken(&$2);
                                                        }
          | andExp                                      { $$ = $1; }
          ;

andExp
          : andExp AND unaryRelExp                      { 
                                                            $$ = createOpNode($2->tokenString, $2->line, $1, $3, NULL); 
                                                            removeToken(&$2);
                                                        }
          | unaryRelExp                                 { $$ = $1; }
          ;

unaryRelExp
          : NOT unaryRelExp                             { 
                                                            $$ = createOpNode($1->tokenString, $1->line, $2, NULL, NULL); 
                                                            removeToken(&$1);
                                                        }
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
          | EQ                                  { $$ = $1; }
          | NEQ                                 { $$ = $1; }
          ;

sumExp
          : sumExp sumop mulExp                 { $$ = createOpNode($2->name, $2->lineNum, $1, $3, NULL); }
          | mulExp                              { $$ = $1; }
          ;

sumop
          : ADD                                 { $$ = createNodeFromToken($1, 4); removeToken(&$1); }
          | SUB                                 { $$ = createNodeFromToken($1, 4); removeToken(&$1); }
          ;

mulExp
          : mulExp mulop unaryExp               { $$ = createOpNode($2->name, $2->lineNum, $1, $3, NULL); }
          | unaryExp                            { $$ = $1; }
          ;

mulop
          : MUL                                 { $$ = createNodeFromToken($1, 4); removeToken(&$1); }
          | DIV                                 { $$ = createNodeFromToken($1, 4); removeToken(&$1); }
          | MOD                                 { $$ = createNodeFromToken($1, 4); removeToken(&$1); }
          ;

unaryExp
          : unaryop unaryExp                    { $$ = $1;  $$->child[0] = $2; }
          | factor                              { $$ = $1; }
          ;

unaryop
          : SUB                                 { $$ = createOpNode("chsign", $1->line, NULL, NULL, NULL); removeToken(&$1); }
          | MUL                                 { $$ = createOpNode("sizeof", $1->line, NULL, NULL, NULL); removeToken(&$1); }
          | RAND                                { $$ = createOpNode($1->tokenString, $1->line, NULL, NULL, NULL); removeToken(&$1); }
          ;

factor
          : mutable                             { $$ = $1; }
          | immutable                           { $$ = $1; }
          ;

mutable
          : ID                                  { $$ = createNodeFromToken($1, 5); removeToken(&$1); }
          | ID '[' expression ']'               { 
                                                    $$ = createOpNode("[", $1->line, createNodeFromToken($1, 5), $3, NULL); 
                                                    removeToken(&$1);
                                                }
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
                                                    $$->name = strdup($1->tokenString);
                                                    $$->lineNum = $1->line;
                                                    $$->num_params = countSiblings($3);   // save number of params
                                                    if($3 != NULL) $$->child[0] = $3;
                                                    removeToken(&$1);
                                                }     
          ;

args
          : argList                             { $$ = $1; }
          | %empty                              { $$ = NULL; }
          ;

argList
          : argList ',' expression              {
                                                    AST_Node* t = $1;
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
                                                    removeToken(&$1);
                                                }
          | CHARCONST                           {
                                                    $$ = createNodeFromToken($1, 6);
                                                    $$->expType = Char;
                                                    removeToken(&$1);
                                                }
          | STRINGCONST                         {
                                                    $$ = createNodeFromToken($1, 6);
                                                    $$->expType = Char;
                                                    $$->varKind = Global;
                                                    $$->isArray = true;
                                                    $$->size = $1->length + 1;
                                                    removeToken(&$1);
                                                }
          | BOOLCONST                           {   
                                                    $$ = createNodeFromToken($1, 6);
                                                    $$->expType = Boolean;
                                                    removeToken(&$1);
                                                }
          ;

%%

// needs to be updated?
void yyerror(const char *s) {
  printf("EEK, parse error on line: %d! Message: %s\n", 111, s);
  // might as well halt now:
  exit(-1);
}