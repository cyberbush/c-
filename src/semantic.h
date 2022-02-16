//------------------ semantic.h ------------------
#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include "scanType.h"
#include "AST_Node.h"
#include "symbolTable.cpp"
#include <iostream>
#include <stdlib.h>
#include <string.h>
using namespace std;

class SemanticAnalyzer {
    private:

        int errNum;                         // number of errors
        int warnNum;                        // number of warnings        
        bool isMain;                        // is there a main
        SymbolTable symTable;               // symbol table used in analysis
        AST_Node *startFunction = NULL;     // store the start of function here


        void traverseTree(AST_Node *root);  // traverses through tree using preorder
        
        void analyNode(AST_Node *root);     // check node

        void analyDecl(AST_Node *n);        // analyze declaration
        void handleVar(AST_Node *n);        // handles Variables
        void handleFunc(AST_Node *n);       // handles Functions
        void handleParam(AST_Node *n);      // handles parameters

        void analyStmt(AST_Node *n);        // analyze statement
        void analyExp(AST_Node *n);         // analyze expression

    public:

        SemanticAnalyzer();   // class constructor
        void analyze(AST_Node* root, bool symTableDebug); // function to analyze the tree

        SymbolTable getSymbolTable();
        int getWarnings();
        int getErrors();

};


#endif