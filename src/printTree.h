//------------------ printTree.h ------------------
#ifndef _PRINTTREE_H_
#define _PRINTTREE_H_

// Functions for printing the AST

// Prints all nodes in the AST in prefix order
static void printAST(AST_Node* root, int childNum, int level);
// Prints all nodes in the sibling AST 
static void printSiblingAST(AST_Node* root, int siblingOrder, int level);
// Used for printing a node, can be a TermK, DeclK, StmtK, or ExpK
static void printNode(AST_Node* n);
// Used for printing terminals
static void printTerm(AST_Node* n);
// Used for printing Decl
static void printDecl(AST_Node* n);
// Used for printing Exp
static void printExp(AST_Node* n);
// Used for printing Stmt
static void printStmt(AST_Node* n);
 
#endif