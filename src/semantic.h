//------------------ semantic.h ------------------
#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include "scanType.h"
#include "AST_Node.h"
#include "symbolTable.cpp"
#include "errorMessages.cpp"
#include "treeUtil.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
using namespace std;

// This class manages the semantic analysis by recursing
// throw the AST and checking for errors and warnings

class SemanticAnalyzer {
    private:

        SymbolTable symTable;               // symbol table used in analysis
        ErrorMessages errors;               // Handles error and warning messages
        AST_Node *startFunction = NULL;     // store the start of function here
        bool isMain;                        // is there a main function

        // traverses through tree using preorder
        // mainly setting up and managing scopes
        void firstTraversal(AST_Node *root);
        
        // traverse through tree using postorder
        // mainly checking for errors now
        void secondTraversal(AST_Node *root);
        
        // functions used in first traversal
        void analyzeNode(AST_Node *root);   
        void analyzeDecl(AST_Node *n);      
        void handleVar(AST_Node *n);   
        void handleFunc(AST_Node *n);       
        void analyzeStmt(AST_Node *n);     
        void handleReturn(AST_Node *n);     
        void analyzeExp(AST_Node *n);      
        void handleId(AST_Node *n);         
        void handleCall(AST_Node *n);      
        void handleInit(AST_Node *n);       

        // helper functions
        void manageScope(AST_Node *n);                              // handles the scope during recursion
        void checkMain(AST_Node *n);                                // checks to see if main function
        void manageUsedVars(map<string, void*> symbols);
        // deal with initializations
        void checkOpChildInit(AST_Node *lhs, AST_Node *rhs, string op);     // check initialization for operator children
        void checkVarSideInit(AST_Node *n);                                 // check if variables on side are initialized
        void checkVarInit(AST_Node *n);                                     // check each variable is initialized
        void initLeftVar(AST_Node *n);                                      //
        void handleReturnInit(AST_Node *n);

        // functions used in second traversal
        void analyzeNodeErrors(AST_Node* n);
        void handleStmtErrors(AST_Node* n);
        void handleExpErrors(AST_Node* n);
        void handleRangeErrors(AST_Node* n);

        // deal with expression assignment and operators
        ExpType findAssOpType(AST_Node* n, ExpKind expK);
        ExpType findBinaryOp(AST_Node* n, string op);
        ExpType findUnaryOp(AST_Node* n, string op);
        void compareBothTypes(ExpType lhs, ExpType rhs, ExpType expected, string op, int line);
        ExpType compareBothNodeTypes(AST_Node *lhs, AST_Node *rhs, string op, int line);

    public:

        SemanticAnalyzer();   // class constructor

        void analyzeTree(AST_Node* root, bool symTableDebug); // function to analyzeze the tree

        SymbolTable getSymbolTable();
};


#endif