%{
#include "main.cpp"
#define YYERROR_VERBOSE
using namespace std;

AST_Node* root;      // root of the tree

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
%type <tree> paramTypeList paramIdList paramId statement matched
%type <tree> unmatched expressionStmt compoundStmt localDecs stmtList
%type <tree> matchedSelectStmt unmatchedSelectStmt matchedIterationStmt
%type <tree> unmatchedIterationStmt iterationRange returnStmt breakStmt
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
          | error                           { $$ = NULL; }
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
                                                yyerrok;
                                            }
          | error varDecList ';'            { $$ = NULL; yyerrok; }
          | typeSpec error ';'              { $$ = NULL; yyerrok; yyerrok; }
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
                                                    t->isInitialized = true; // static variables automatically Initialized
                                                    t = t->sibling;
                                                }
                                                yyerrok; 
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
                                                yyerrok;
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
                                                yyerrok;
                                            }
          | varDecList ',' error            { $$ = NULL; }
          | error                           { $$ = NULL; }
          | varDecInit                      { $$ = $1; }
          ;

varDecInit
          : varDecId                        { $$ = $1; }
          | varDecId ':' simpleExp          {
                                                $$ = $1;
                                                if($3 != NULL && $$ != NULL){
                                                    $$->child[0] = $3;
                                                    $$->isInitialized = true;
                                                    //$$->isDeclUsed = true;
                                                    $$->hasInit = true;
                                                }
                                            }
          | error ':' simpleExp             { $$ = NULL; yyerrok; }
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
          | ID '[' error                    { $$ = NULL; }
          | error ']'                       { $$ = NULL; yyerrok; }
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
          | typeSpec error                                  { $$ = NULL; }
          | typeSpec ID '(' error                           { $$ = NULL; }
          | ID '(' error                                    { $$ = NULL; }
          | ID '(' params ')' error                         { $$ = NULL; }
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
          | paramList ';' error             { $$ = NULL; }
          | error                           { $$ = NULL; }
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
          | typeSpec error                  { $$ = NULL; }
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
                                                yyerrok;
                                            }
          | paramId                         { $$ = $1; }
          | paramIdList ',' error           { $$ = NULL; }
          | error                           { $$ = NULL; }
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
          : matched                          { $$ = $1; }
          | unmatched                        { $$ = $1; }
          ;

matched
          : expressionStmt                      { $$ = $1; }
          | compoundStmt                        { $$ = $1; }
          | returnStmt                          { $$ = $1; }
          | breakStmt                           { $$ = $1; }
          | matchedSelectStmt                   { $$ = $1; }
          | matchedIterationStmt                { $$ = $1; }
	      | IF error                            { $$ = NULL; }
	      | IF error ELSE matched               { $$ = NULL; yyerrok; }
	      | IF error THEN matched ELSE matched  { $$ = NULL; yyerrok; }
	      | WHILE error DO matched              { $$ = NULL; yyerrok; }
	      | WHILE error                         { $$ = NULL; }
	      | FOR ID ASGN error DO matched        { $$ = NULL; yyerrok; }
	      | FOR error                           { $$ = NULL; }
          ;

unmatched
          : unmatchedSelectStmt                 { $$ = $1; }
          | unmatchedIterationStmt              { $$ = $1; }
          | IF error THEN statement             { $$ = NULL; yyerrok; }
          | IF error ELSE unmatched             { $$ = NULL; yyerrok; }
          | IF error THEN matched ELSE unmatched { $$ = NULL; yyerrok; }
          | WHILE error DO unmatched            { $$ = NULL; yyerrok; }
          | FOR ID ASGN error DO unmatched      { $$ = NULL; yyerrok; }
          ;

expressionStmt
          : expression ';'                  { $$ = $1; }
          | ';'                             { $$ = NULL; }
          | error ';'                       { $$ = NULL; yyerrok; }
          ;

compoundStmt
          : BEG localDecs stmtList END      {
                                                $$ = createStmtNode(CompoundK, "", $1->line, $2, $3, NULL);
                                                $$->name = strdup("comp scope");
                                                yyerrok;
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
matchedSelectStmt
          : IF simpleExp THEN matched ELSE matched        { 
                                                                    $$ = createStmtNode(IfK, "if", $1->line, $2, $4, $6); 
                                                                    removeToken(&$1);
                                                                    removeToken(&$3);
                                                                    removeToken(&$5);
                                                                }
          ;

unmatchedSelectStmt
          : IF simpleExp THEN statement                         { 
                                                                    $$ = createStmtNode(IfK, "if", $1->line, $2, $4, NULL); 
                                                                    removeToken(&$1);
                                                                    removeToken(&$3);
                                                                }
          | IF simpleExp THEN matched ELSE unmatched          { 
                                                                    $$ = createStmtNode(IfK, "if", $1->line, $2, $4, $6); 
                                                                    removeToken(&$1);
                                                                    removeToken(&$3);
                                                                    removeToken(&$5);
                                                                }
          ;

matchedIterationStmt
          : WHILE simpleExp DO matched                       { 
                                                                    $$ = createStmtNode(WhileK, "", $1->line, $2, $4, NULL); 
                                                                    removeToken(&$1);
                                                                    removeToken(&$3);
                                                                }
          | FOR ID ASGN iterationRange DO matched            {
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

unmatchedIterationStmt
          : WHILE simpleExp DO unmatched                         { 
                                                                    $$ = createStmtNode(WhileK, "", $1->line, $2, $4, NULL); 
                                                                    removeToken(&$1);
                                                                    removeToken(&$3);
                                                                }
          | FOR ID ASGN iterationRange DO unmatched              {
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
          | simpleExp TO error                          { $$ = NULL; }
          | error BY error                              { $$ = NULL; yyerrok; }
          | simpleExp TO simpleExp BY error             { $$ = NULL; }
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
                                                            yyerrok;
                                                            removeToken(&$1);
                                                        }
          | RETURN error ';'                            { $$ = NULL; yyerrok; }                    
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
          | error assignop expression                          { $$ = NULL; yyerrok; }
          | mutable assignop error                      { $$ = NULL; }
          | error INC                                   { $$ = NULL; yyerrok; }
          | error DEC                                   { $$ = NULL; yyerrok; }
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
          | simpleExp OR error                          { $$ = NULL; }
          ;

andExp
          : andExp AND unaryRelExp                      { 
                                                            $$ = createOpNode($2->tokenString, $2->line, $1, $3, NULL); 
                                                            removeToken(&$2);
                                                        }
          | unaryRelExp                                 { $$ = $1; }
          | andExp AND error                            { $$ = NULL; }
          ;

unaryRelExp
          : NOT unaryRelExp                             { 
                                                            $$ = createOpNode($1->tokenString, $1->line, $2, NULL, NULL); 
                                                            removeToken(&$1);
                                                        }
          | relExp                                      { $$ = $1; }
          | NOT error                                   { $$ = NULL; }
          ;

relExp
          : sumExp relop sumExp                         { $$ = createOpNode($2->tokenString, $2->line, $1, $3, NULL); }
          | sumExp                                      { $$ = $1; }
          | sumExp relop error                          { $$ = NULL; }
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
          | sumExp sumop error                  { $$ = NULL; }
          ;

sumop
          : ADD                                 { $$ = createNodeFromToken($1, 4); removeToken(&$1); }
          | SUB                                 { $$ = createNodeFromToken($1, 4); removeToken(&$1); }
          ;

mulExp
          : mulExp mulop unaryExp               { $$ = createOpNode($2->name, $2->lineNum, $1, $3, NULL); }
          | unaryExp                            { $$ = $1; }
          | mulExp mulop error                  { $$ = NULL; }
          ;

mulop
          : MUL                                 { $$ = createNodeFromToken($1, 4); removeToken(&$1); }
          | DIV                                 { $$ = createNodeFromToken($1, 4); removeToken(&$1); }
          | MOD                                 { $$ = createNodeFromToken($1, 4); removeToken(&$1); }
          ;

unaryExp
          : unaryop unaryExp                    { $$ = $1;  $$->child[0] = $2; }
          | factor                              { $$ = $1; }
          | unaryop error                       { $$ = NULL; }
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
          : '(' expression ')'                  { $$ = $2; yyerrok; }
          | call                                { $$ = $1; }
          | constant                            { $$ = $1; }
          | '(' error                           { $$ = NULL; }
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
          | error '('                           { $$ = NULL; yyerrok; }     
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
                                                    yyerrok;
                                                }
          | expression                          { $$ = $1; }
          | argList ',' error                   { $$ = NULL; }
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