//------------------ printTree.h ------------------
#ifndef _PRINTTREE_H_
#define _PRINTTREE_H_

// Functions for printing the AST Tree

// Prints all nodes in the AST in prefix order
static void printAST(AST_Tree* root, int childNum, int level);
// Prints all nodes in the sibling AST 
static void printSiblingAST(AST_Tree* root, int siblingOrder, int level);
// Used for printing a node, can be a TermK, DeclK, StmtK, or ExpK
static void printNode(AST_Tree* n);
// Used for printing terminals
static void printTerm(AST_Tree* n);
// Used for printing Decl
static void printDecl(AST_Tree* n);
// Used for printing Exp
static void printExp(AST_Tree* n);
// Used for printing Stmt
static void printStmt(AST_Tree* n);
 
#endif