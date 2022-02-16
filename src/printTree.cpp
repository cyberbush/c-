//------------------ printTree.cpp ------------------

#include "printTree.h"

void printAST(AST_Node* root, int childNum, int level, bool annotated)
{
    if(root == NULL) return;
    // Print the indention level
    for(int i = 0; i < level; i++)
    {
        printf(".   ");
    }
    if(childNum >= 0) printf("Child: %i  ", childNum);
    printNode(root);  // Print root at same level
    printf(" [line: %i]\n", root->lineNum);
    for(int i = 0; i < 3; i++)  // Print everything below the child
    {
      if(root->child[i] != NULL){
          printAST(root->child[i], i, level+1, annotated);
      }
    }

    if(root->sibling != NULL)
    {
        printSiblingAST(root->sibling, 1, level, annotated); // Print the sibling tree
    }
}

void printSiblingAST(AST_Node* root, int siblingOrder, int level, bool annotated)
{
    // Print the indention level
    for(int i = 0; i < level; i++)
    {
        printf(".   ");
    }
    // print sibling index
    printf("Sibling: %i  ", siblingOrder);
    printNode(root);  // Print root at same level
    printf(" [line: %i]\n", root->lineNum);
    for(int i = 0; i < 3; i++)  // Print everything below the child
    {
      if(root->child[i] != NULL){
        printAST(root->child[i], i, level+1, annotated);
      }
    }

    if(root->sibling != NULL)
    {
        printSiblingAST(root->sibling, ++siblingOrder, level, annotated); // Print the sibling tree
    }
}

void printNode(AST_Node* n)
{
    if(n == NULL) return;
    switch(n->nodeKind)
    {
        case DeclK:
            printDecl(n);
            break;
        case StmtK:
            printStmt(n);
            break;
        case ExpK:
            printExp(n);
            break;
        case TermK:
            printTerm(n);
            break;
        default:
            printf("Error: NodeKind undefined\n");
            break;
    }
}

void printTerm(AST_Node* n)
{
    switch(n->subkind.term)
    {
        case IDD:
            printf("Id: %s", n->printName);
            break;

        default:
            printf("Error: Unknown TermKind\n");
            break;
    }
}

void printDecl(AST_Node* n)
{
    switch(n->subkind.decl)
    {
        case VarK:
            if(n->isArray) printf("Var: %s of array of type %s", n->printName, ExpTypeToStr(n->expType));
            else if(n->isStatic) printf("Var: %s of static type %s", n->printName, ExpTypeToStr(n->expType));
            else printf("Var: %s of type %s", n->printName, ExpTypeToStr(n->expType));
            break;        
        case FuncK:
            printf("Func: %s returns type %s", n->printName, ExpTypeToStr(n->expType));
            break;
        case ParamK:
            if(n->isArray) {printf("Parm: %s of array of type %s", n->printName, ExpTypeToStr(n->expType));}
            else {printf("Parm: %s of type %s", n->printName, ExpTypeToStr(n->expType));}
            break;
        default:
            printf("Error: unknown subtype of DECL\n");
            break;
    }
}

void printStmt(AST_Node* n)
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

void printExp(AST_Node* n)
{
    switch(n->subkind.exp)
    {
        case ConstantK:
            printf("Const %s", n->printName);
            break;
        case IdK:
            printf("Id: %s", n->printName);
            break;
        case AssignK:
            printf("Assign: %s", n->printName);
            break;
        case OpK:
            printf("Op: %s", n->printName);
            break;
        case CallK:
            printf("Call: %s", n->printName);
            break;
        case InitK:
            break;
        default:
            printf("Error: Unknown ExpKind\n");
            break;
    }
}
