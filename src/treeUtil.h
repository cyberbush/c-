//------------------ treeUtil.h ------------------
#ifndef _TREEUTIL_H_
#define _TREEUTIL_H_
/*
    Utility functions for managing the tree
*/

// Creating Node Functions
static AST_Node* createNode(NodeKind);
static AST_Node* createNodeFromToken(TokenData*, int);
static AST_Node* createOpNode(const char*, int, AST_Node*, AST_Node*, AST_Node*);
static AST_Node* createStmtNode(StmtKind, const char*, int, AST_Node*, AST_Node*, AST_Node*);
static AST_Node* createDeclNode(DeclKind, ExpType, const char*, int, AST_Node*, AST_Node*, AST_Node*);

#endif