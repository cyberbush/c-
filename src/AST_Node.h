//------------------ AST_Node Header ------------------
#ifndef _AST_Node_H_
#define _AST_Node_H_

#include <stdio.h>
#include <string>
#include <map>
using namespace std;

// The max children that a tree node can have
#define MAX_CHILDREN 3

//------------------- Type Definitions -------------------
// Kinds of Operators these are the token numbers for the operators same as in flex
typedef int OpKind;
enum NodeKind {DeclK, StmtK, ExpK, TermK}; // Kinds of Statements
enum DeclKind {VarK, FuncK, ParamK}; // Subkinds of Declarations
enum StmtKind {NullK, IfK, WhileK, ForK, CompoundK, ReturnK, BreakK, RangeK}; // Subkinds of Statements
enum ExpKind {OpK, ConstantK, IdK, AssignK, InitK, CallK}; // Subkinds of Expressions
// ExpType is used for type checking (Void means no type or value, UndefinedType means undefined)
enum ExpType {Void, Integer, Boolean, Char, CharInt, Equal, UndefinedType};
// What kind of scoping is used? (decided during typing)
enum VarKind {None, Local, Global, Parameter, LocalStatic};
// What type of terminal
enum TermKind {NUMC, DECL, IDD};

typedef struct AST_Node AST_Node;
struct AST_Node {

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
    AST_Node *child[MAX_CHILDREN];  // Node can have a max of 3 children but
    AST_Node *sibling;              // Only 1 sibling
    
    AST_Node *firstDecl =NULL;      // keeps track of first declaration

    //------------------- Variables -------------------
    NodeKind nodeKind;              // Type of this node 
    union subkind                   // subtype of type
    {
        DeclKind decl;              // used when DeclK
        StmtKind stmt;              // used when StmtK
        ExpKind exp;                // used when ExpK
        TermKind term;              // used for type of terminal
    } subkind;

    union attrib                    // attributes of this node
    {
        OpKind op;                  // type of token (same as in bison)
        int value;                  // used when an integer constant or boolean
        unsigned char cvalue;       // used when a character
        const char* str;            // used when a string constant
    } attrib;

    char* name;                     // used to store the name of the node

    ExpType expType;                // used when ExpK for type checking
    VarKind varKind = None;         // type of variable

    int num_params =  0;            // store the number of paramaters

    map<int, pair<ExpType, bool>> params;

    int lineNum;                    // Line number associated with this node

    bool isInitialized = false;     // is it initialized?
    bool isStatic = false;          // is staticly allocated?
    bool isArray = false;           // is this an array
    bool isMain = false;            // keep track of main node
    bool isDeclUsed = false;        // check if declaration has been used
    bool hasReturn = false;         // check if function has a return
    bool hasInit = true;            //
    bool isSpecialC = false;        // Used for special characters like '\0', '\t', '\n' ect..

    int size = 0;                   // value size in words
    int stackLocation = 1;          //
};

#endif