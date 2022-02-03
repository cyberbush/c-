//------------------ utils.h ------------------
#ifndef _UTILS_H_
#define _UTILS_H_
/*
*   The utils file will be used for utility functions
*   such as printing, building nodes, ect.
*/


// Creating Node Functions
static AST_Tree* createNode(NodeKind);
static AST_Tree* createNodeFromToken(TokenData*, int);
static AST_Tree* createOpNode(const char*, int, AST_Tree*, AST_Tree*, AST_Tree*);
static AST_Tree* createStmtNode(StmtKind, const char*, int, AST_Tree*, AST_Tree*, AST_Tree*);
static AST_Tree* createDeclNode(DeclKind, ExpType, const char*, int, AST_Tree*, AST_Tree*, AST_Tree*);

// Function to count number of arguments for a AST Tree node
int countSiblings(AST_Tree*);
// Returns the sibling to the far right
AST_Tree* getLastSibling(AST_Tree*);
// Takes an ExpType and returns the corresponding string
const char* ExpTypeToStr(ExpType type);



#endif
