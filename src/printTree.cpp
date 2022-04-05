//------------------ printTree.cpp ------------------

#include "printTree.h"

void printAST(AST_Node* root, int childNum, int level, bool PFlag)
{
    if(root == NULL) return;
    // Print the indention level
    for(int i = 0; i < level; i++)
    {
        printf(".   ");
    }
    if(childNum >= 0) printf("Child: %i  ", childNum);
    printNode(root, PFlag);  // Print root at same level
    printf(" [line: %i]\n", root->lineNum);
    for(int i = 0; i < 3; i++)  // Print everything below the child
    {
      if(root->child[i] != NULL){
          printAST(root->child[i], i, level+1, PFlag);
      }
    }

    if(root->sibling != NULL)
    {
        printSiblingAST(root->sibling, 1, level, PFlag); // Print the sibling tree
    }
}

void printSiblingAST(AST_Node* root, int siblingOrder, int level, bool PFlag)
{
    // Print the indention level
    for(int i = 0; i < level; i++)
    {
        printf(".   ");
    }
    // print sibling index
    printf("Sibling: %i  ", siblingOrder);
    printNode(root, PFlag);  // Print root at same level
    printf(" [line: %i]\n", root->lineNum);
    for(int i = 0; i < 3; i++)  // Print everything below the child
    {
      if(root->child[i] != NULL){
        printAST(root->child[i], i, level+1, PFlag);
      }
    }

    if(root->sibling != NULL)
    {
        printSiblingAST(root->sibling, ++siblingOrder, level, PFlag); // Print the sibling tree
    }
}

void printNode(AST_Node* n, bool PFlag)
{
    if(n == NULL) return;
    switch(n->nodeKind)
    {
        case DeclK:
            printDecl(n, PFlag);
            break;
        case StmtK:
            printStmt(n, PFlag);
            break;
        case ExpK:
            printExp(n, PFlag);
            break;
        case TermK:
            printTerm(n, PFlag);
            break;
        default:
            printf("Error: NodeKind undefined\n");
            break;
    }
}

void printTerm(AST_Node* n, bool PFlag)
{
    switch(n->subkind.term)
    {
        case IDD:
            printf("Id: %s", n->name);
            if(PFlag) { printf(" of type %s", ExpTypeToStr(n->expType));}
            break;

        default:
            printf("Error: Unknown TermKind\n");
            break;
    }
}

void printDecl(AST_Node* n, bool PFlag)
{
    string statc = n->isStatic ? "static " : "";
    switch(n->subkind.decl)
    {
        case VarK:
            if(n->isArray) { 
                if(PFlag) { printf("Var: %s is array of type %s", n->name, ExpTypeToStr(n->expType)); }
                else { printf("Var: %s is array of type %s", n->name, ExpTypeToStr(n->expType)); }  
            }
            else if(n->isStatic) {
                //if(PFlag) { printf("Var: %s of %stype %s", n->name, statc.c_str(), ExpTypeToStr(n->expType)); }
                printf("Var: %s of type %s", n->name, ExpTypeToStr(n->expType));
            }
            else {
                if(PFlag) { printf("Var: %s of type %s", n->name, ExpTypeToStr(n->expType)); }
                else { printf("Var: %s of type %s", n->name, ExpTypeToStr(n->expType)); }
            }
            break;        
        case FuncK:
            printf("Func: %s returns type %s", n->name, ExpTypeToStr(n->expType));
            break;
        case ParamK:
            if(n->isArray) {printf("Parm: %s is array of type %s", n->name, ExpTypeToStr(n->expType));}
            else {printf("Parm: %s of type %s", n->name, ExpTypeToStr(n->expType));}
            break;
        default:
            printf("Error: unknown subtype of DECL\n");
            break;
    }
}

void printStmt(AST_Node* n, bool PFlag)
{
    switch(n->subkind.stmt)
    {
        case ReturnK:
            printf("Return");
            break;
        case BreakK:
            printf("Break");
            break;
        case CompoundK:
            printf("Compound");
            break;
        case IfK:
            printf("If");
            break;
        case WhileK:
            printf("While");
            break;
        case ForK:
            printf("For");
            break;
        case RangeK:
            printf("Range");
            break;
        default:
            printf("Error: Unknown StmtKind\n");
    }
}

void printExp(AST_Node* n, bool PFlag)
{
    string statc = n->isStatic ? "static " : "";
    string array = n->isArray ? "is array " : "";
    string mem_info = n->isArray ? to_string(n) : "";
    switch(n->subkind.exp)
    {
        case ConstantK:
            if(PFlag) { if(!n->isSpecialC) { printf("Const %s", array.c_str()); printNoLeadingZero(n->name); printf(" of type %s", ExpTypeToStr(n->expType));}
                        else { printf("Const %s'%c' of type %s", array.c_str(), (char)n->attrib.cvalue, ExpTypeToStr(n->expType)); } }
            else { printf("Const %s", n->name); }
            break;
        case IdK:
            printf("Id: %s", n->name);
            if(PFlag) { printf(" of type %s", ExpTypeToStr(n->expType)); }
            break;
        case AssignK:
            printf("Assign: %s", n->name);
            if(PFlag) { printf(" of type %s", ExpTypeToStr(n->expType)); }
            break;
        case OpK:
            printf("Op: %s", n->name);
            if(PFlag) { if(string(n->name) == "=") { printf(" of type bool"); } else { printf(" of type %s", ExpTypeToStr(n->expType)); }}
            break;
        case CallK:
            printf("Call: %s", n->name);
            if(PFlag) { printf(" of type %s", ExpTypeToStr(n->expType)); }
            break;
        case InitK:
            break;
        default:
            printf("Error: Unknown ExpKind\n");
            break;
    }
}

// Print augmented tree with memory information 
void printASTAugmented(AST_Node* root, int childNum, int level)
{
    if(root == NULL) return;
    // Print the indention level
    for(int i = 0; i < level; i++)
    {
        printf(".   ");
    }
    if(childNum >= 0) printf("Child: %i  ", childNum);
    printNodeAug(root);  // Print root at same level
    printf(" [line: %i]\n", root->lineNum);
    for(int i = 0; i < 3; i++)  // Print everything below the child
    {
      if(root->child[i] != NULL){
          printASTAugmented(root->child[i], i, level+1);
      }
    }

    if(root->sibling != NULL)
    {
        printSiblingASTAug(root->sibling, 1, level); // Print the sibling tree
    }
}

void printSiblingASTAug(AST_Node* root, int siblingOrder, int level)
{
    // Print the indention level
    for(int i = 0; i < level; i++)
    {
        printf(".   ");
    }
    // print sibling index
    printf("Sibling: %i  ", siblingOrder);
    printNodeAug(root);  // Print root at same level
    printf(" [line: %i]\n", root->lineNum);
    for(int i = 0; i < 3; i++)  // Print everything below the child
    {
      if(root->child[i] != NULL){
        printASTAugmented(root->child[i], i, level+1);
      }
    }

    if(root->sibling != NULL)
    {
        printSiblingASTAug(root->sibling, ++siblingOrder, level); // Print the sibling tree
    }
}

void printNodeAug(AST_Node* n)
{
    if(n == NULL) return;
    switch(n->nodeKind)
    {
        case DeclK:
            printDeclAug(n);
            break;
        case StmtK:
            printStmtAug(n);
            break;
        case ExpK:
            printExpAug(n);
            break;
        case TermK:
            printTermAug(n);
            break;
        default:
            printf("Error: NodeKind undefined\n");
            break;
    }
}

void printTermAug(AST_Node* n)
{
    switch(n->subkind.term)
    {
        case IDD:
            printf("Id: %s of type %s", n->name, ExpTypeToStr(n->expType));
            break;

        default:
            printf("Error: Unknown TermKind\n");
            break;
    }
}

void printDeclAug(AST_Node* n)
{
    string statc = n->isStatic ? " static " : "";
    string array = n->isArray ? " of array" : "";
    if(n->isStatic && n->isArray) {
        statc = " of static array";
        array = "";
    }
    switch(n->subkind.decl)
    {
        case VarK:
            printf("Var: %s%s%s of type %s", n->name, statc.c_str(), array.c_str(), ExpTypeToStr(n->expType));
            break;        
        case FuncK:
            printf("Func: %s returns type %s", n->name, ExpTypeToStr(n->expType));
            break;
        case ParamK:
            if(n->isArray) {printf("Parm: %s of array of type %s", n->name, ExpTypeToStr(n->expType));}
            else {printf("Parm: %s of type %s", n->name, ExpTypeToStr(n->expType));}
            break;
        default:
            printf("Error: unknown subtype of DECL\n");
            break;
    }
    // print memory info
    printMemory(n, false);
}

void printStmtAug(AST_Node* n)
{
    switch(n->subkind.stmt)
    {
        case ReturnK:
            printf("Return");
            break;
        case BreakK:
            printf("Break");
            break;
        case CompoundK:
            printf("Compound");
            printMemory(n, false);
            break;
        case IfK:
            printf("If");
            break;
        case WhileK:
            printf("While");
            break;
        case ForK:
            if(n->varKind != None) { printf("For %s", to_string(n).c_str()); }
            else { printf("For%s", to_string(n).c_str()); }
            break;
        case RangeK:
            printf("Range");
            break;
        default:
            printf("Error: Unknown StmtKind\n");
    }
}

void printExpAug(AST_Node* n)
{
    string statc = n->isStatic ? " static" : "";
    string array = n->isArray ? " of array" : "";
    if(n->isStatic && n->isArray) {
        statc = " of static array";
        array = "";
    }
    string mem_info = n->isArray ? to_string(n) : "";
    switch(n->subkind.exp)
    {
        case ConstantK:
            if(!n->isSpecialC) { printf("Const %s", array.c_str()); printNoLeadingZero(n->name); printf(" of type %s%s", ExpTypeToStr(n->expType), mem_info.c_str());}
            else { printf("Const %s'%c' of type %s%s", array.c_str(), (char)n->attrib.cvalue, ExpTypeToStr(n->expType), mem_info.c_str()); } 
            break;
        case IdK:
            printf("Id: %s%s%s of type %s", n->name, statc.c_str(), array.c_str(), ExpTypeToStr(n->expType));
            printMemory(n, false);
            break;
        case AssignK:
            printf("Assign: %s of type %s", n->name, ExpTypeToStr(n->expType));
            break;
        case OpK:
            printf("Op: %s", n->name);
            if(string(n->name) == "=") { printf(" of type bool"); } 
            else { printf(" of type %s", ExpTypeToStr(n->expType)); }
            break;
        case CallK:
            printf("Call: %s of type %s", n->name, ExpTypeToStr(n->expType));
            break;
        case InitK:
            break;
        default:
            printf("Error: Unknown ExpKind\n");
            break;
    }
}

void printNoLeadingZero(const char* str)
{
    int len = strlen(str);
    if(str[0] != '0' || len==1) {
        printf("%s", str);
    }
    else {
        int i = 0;
        while(str[i] == '0' && i<len-1)
            i++;
        while(i<len) {
            printf("%c",str[i]);
            i++;
        }
    }
}