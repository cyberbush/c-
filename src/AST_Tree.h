//------------------ AST_Tree Header ------------------
#ifndef _AST_TREE_H_
#define _AST_TREE_H_

#include <stdio.h>
#include <string>

// The max children that a tree node can have
#define MAX_CHILDREN 3

//------------------- Type Definitions -------------------
// Kinds of Operators these are the token numbers for the operators same as in flex
typedef int OpKind;
enum NodeKind {DeclK, StmtK, ExpK, TerminalK}; // Kinds of Statements
enum DeclKind {VarK, FuncK, ParamK}; // Subkinds of Declarations
enum StmtKind {NullK, IfK, WhileK, ForK, CompoundK, ReturnK, BreakK, RangeK}; // Subkinds of Statements
enum ExpKind {OpK, ConstantK, IdK, AssignK, InitK, CallK}; // Subkinds of Expressions
// ExpType is used for type checking (Void means no type or value, UndefinedType means undefined)
enum ExpType {Void, Integer, Boolean, Char, CharInt, Equal, UndefinedType};
// What kind of scoping is used? (decided during typing)
enum VarKind {None, Local, Global, Parameter, LocalStatic};
// What type of terminal
enum TermKind {NUMC, DECL, IDD};

typedef struct AST_Tree AST_Tree;
struct AST_Tree {

public:
    /* 
	*   AST Tree Structure:
    *
	* 		   Parent Node		-> 		Sibling
	*	      /     |     \
	*	Child 0  Child 1  Child 2
    *
    */
    // Related nodes
    AST_Tree *child[MAX_CHILDREN]; // Node can have a max of 3 children but
    AST_Tree *sibling; // Only 1 sibling

    //------------------- Variables -------------------
    NodeKind nodeKind; // Type of this node 
    union subkind// subtype of type
    {
        DeclKind decl; // used when DeclK
        StmtKind stmt; // used when StmtK
        ExpKind exp; // used when ExpK
        TermKind term; // used for type of terminal
    } subkind;

    union attrib// attributes of this node
    {
        OpKind op; // type of token (same as in bison)
        int value; // used when an integer constant or boolean
        unsigned char cvalue; // used when a character
        const char* str; // used when a string constant
        const char* name; // used when IdK
    } attrib;

    ExpType expType; // used when ExpK for type checking
    VarKind varKind = None; // type of variable

    int num_params =  0; // store the number of paramaters
    int lineNum; // Line number associated with this node

    bool isArray; // is this an array
    bool isStatic; // is staticly allocated?
    bool isInitialized; // is it initialized?
};

// Creating Node Functions
static AST_Tree* createNode(NodeKind);
static AST_Tree* createNodeFromToken(TokenData*, int);
static AST_Tree* createOpNode(const char*, int, AST_Tree*, AST_Tree*, AST_Tree*);
static AST_Tree* createStmtNode(StmtKind, const char*, int, AST_Tree*, AST_Tree*, AST_Tree*);
static AST_Tree* createDeclNode(DeclKind, ExpType, const char*, int, AST_Tree*, AST_Tree*, AST_Tree*);


#endif