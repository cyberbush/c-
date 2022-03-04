//------------------ treeUtil.h ------------------
#ifndef _TREEUTIL_H_
#define _TREEUTIL_H_

#include "scanType.h"
#include "AST_Node.h"
#include "utils.h"
#include "string.h"

/*
    Utility functions for managing the tree
*/

// Creating Node Functions
static AST_Node* createNode(NodeKind);
static AST_Node* createNodeFromToken(TokenData*, int);
static AST_Node* createOpNode(const char*, int, AST_Node*, AST_Node*, AST_Node*);
static AST_Node* createStmtNode(StmtKind, const char*, int, AST_Node*, AST_Node*, AST_Node*);
static AST_Node* createDeclNode(DeclKind, ExpType, const char*, int, AST_Node*, AST_Node*, AST_Node*);

// Copy data from one to another
static void copyNodeData(AST_Node* n1, AST_Node* n2);

// Check if node is ID and Array
bool isNodeID_Array(AST_Node *n);
#endif