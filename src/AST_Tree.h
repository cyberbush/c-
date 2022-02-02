//------------------ AST_Tree Header ------------------
#ifndef _AST_TREE_H_
#define _AST_TREE_H_

#include <stdio.h>
#include <string>

// The max children that a tree node can have
#define MAX_CHILDREN 3

//------------------- Data Structures for Types -------------------
// Kinds of Operators these are the token numbers for the operators same as in flex
typedef int OpKind;
enum NodeKind {DeclK, StmtK, ExpK}; // Kinds of Statements
enum DeclKind {VarK, FuncK, ParamK}; // Subkinds of Declarations
enum StmtKind {NullK, IfK, WhileK, ForK, CompoundK, ReturnK, BreakK, RangeK}; // Subkinds of Statements
enum ExpKind {OpK, ConstantK, IdK, AssignK, InitK, CallK}; // Subkinds of Expressions
// ExpType is used for type checking (Void means no type or value, UndefinedType means undefined)
enum ExpType {Void, Integer, Boolean, Char, CharInt, Equal, UndefinedType};
// What kind of scoping is used? (decided during typing)
enum VarKind {None, Local, Global, Parameter, LocalStatic};

typedef struct AST_Tree {

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
    int lineNum; // Line number associated with this node
    NodeKind nodekind; // Type of this node 
    union subkind// subtype of type
    {
        DeclKind decl; // used when DeclK
        StmtKind stmt; // used when StmtK
        ExpKind exp; // used when ExpK
    } subkind;

    union attrib// attributes of this node
    {
        OpKind op; // type of token (same as in bison)
        int value; // used when an integer constant or boolean
        unsigned char cvalue; // used when a character
        std::string str; // used when a string constant
        std::string name; // used when IdK
    } attrib;

    ExpType expType; // used when ExpK for type checking
    bool isArray; // is this an array
    bool isStatic; // is staticly allocated?
};

#endif