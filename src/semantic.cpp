//------------------ semantic.cpp ------------------
#include "semantic.h"

// STATEMENTS
// create a new scope for each compound statement. make sure to leave scope
// functions CANT return an entire array
// check void functions dont return anything
// check to make sure there is a main function

// DECLARATIONS
// check for duplicate declarations, use symbol table
// check to see if a variable has been declared
// issue a warning if variable is being used before it was initialized (only local variables)
// 

// EXPRESSIONS
// check assignment and other operators to make sure types are correct
// ExpType childTypes[] = {UndefinedType, UndefinedType, UndefinedType}

SemanticAnalyzer::SemanticAnalyzer()
{
    errNum = 0;
    warnNum = 0;
    isMain = false;
}

// start the semantic analysis
void SemanticAnalyzer::analyze(AST_Node *root, bool symTableDebug)
{
    if(symTableDebug) symTable.debug(true); // set symbol table debugging
    traverseTree(root);
    if(!isMain) { errNum++; printf("ERROR(LINKER): A function named 'main()' must be defined.\n"); }
}

// Traverse through each child and sibling in tree (preorder)
void SemanticAnalyzer::traverseTree(AST_Node *root)
{
    if(root==NULL) return;

    // start analyzing node
    analyNode(root);

    // check children
    for(int i=0; i<3; i++) {
        if(root->child[i] != NULL) { traverseTree(root->child[i]); }
    }
    
    // check sibling
    if(root->sibling != NULL) { traverseTree(root->sibling); }
}

// check node for its type
void SemanticAnalyzer::analyNode(AST_Node *n)
{
    switch(n->nodeKind) {
        case DeclK:
            analyDecl(n);
            break;
        case StmtK:
            //analyStmt(n);
            break;
        case ExpK:
            //analyExp(n);
            break;
        default:
            printf("Error in SemanticAnalyzer::analyNode determining type\n");
            break;
    }
}

// Function will analyze each type of declaration
void SemanticAnalyzer::analyDecl(AST_Node *n)
{
    DeclKind declKind = n->subkind.decl;
    switch(declKind) {
        case VarK:
            handleVar(n);
            break;
        case ParamK:
            handleParam(n);
            break;
        case FuncK:
            handleFunc(n);
            break;
        default:
            printf("Error in SemanticAnalyzer::analyDecl determining type\n");
            break;
    }

}

//
void SemanticAnalyzer::handleVar(AST_Node *n)
{
    int line = n->lineNum;
    string name = string(n->printName);

    bool set;
    
    // try to insert into the symbol table
    if(n->isStatic) set = symTable.insertGlobal(name, n);   
    else set = symTable.insert(name, n);
    /*
    if (set) { // inserted successfully

    }
    else { // symbol already added

    }
    */
}

//
void SemanticAnalyzer::handleParam(AST_Node *n)
{
    printf("not complete\n");
}

//
void SemanticAnalyzer::handleFunc(AST_Node *n)
{
    printf("not complete\n");
}

