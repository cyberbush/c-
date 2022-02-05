//------------------ utils.h ------------------
#ifndef _UTILS_H_
#define _UTILS_H_
/*
*   The utils file will be used for utility functions
*   such as printing, building nodes, ect.
*/


// Creating Node Functions
static AST_Node* createNode(NodeKind);
static AST_Node* createNodeFromToken(TokenData*, int);
static AST_Node* createOpNode(const char*, int, AST_Node*, AST_Node*, AST_Node*);
static AST_Node* createStmtNode(StmtKind, const char*, int, AST_Node*, AST_Node*, AST_Node*);
static AST_Node* createDeclNode(DeclKind, ExpType, const char*, int, AST_Node*, AST_Node*, AST_Node*);

// Function to count number of arguments for a AST Tree node
int countSiblings(AST_Node* t);
// Returns the sibling to the far right
AST_Node* getLastSibling(AST_Node* t);
// Takes an ExpType and returns the corresponding string
const char* ExpTypeToStr(ExpType type);



#endif
