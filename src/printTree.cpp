//------------------ printTree.cpp ------------------
#include "printTree.h"
#include "AST_Tree.h"
#include "utils.h"

void printAST(AST_Tree* root, int childNum, int level)
{
    if(root == NULL) return;
    // print indent level
    for(int i = 0; i < level; i++)
    {
        printf(".   ");
    }
    if(childNum >= 0) printf("Child: %i  ", childNum);
    printNode(root);  // print root on same line
    printf(" [line: %i]\n", root->lineNum);
    for(int i = 0; i < 3; i++)  // print trees below children
    {
      if(root->child[i] != NULL){
          printAST(root->child[i], i, level+1);
      }
    }

    if(root->sibling != NULL)
    {
        printSiblingAST(root->sibling, 1, level); // if there is a sibling print it
    }
}

void printSiblingAST(AST_Tree* root, int siblingOrder, int level)
{
    // print indent level
    for(int i = 0; i < level; i++)
    {
        printf(".   ");
    }
    // print sibling index
    printf("Sibling: %i  ", siblingOrder);
    printNode(root);  // print root on same line
    printf(" [line: %i]\n", root->lineNum);
    for(int i = 0; i < 3; i++)  // print trees below children
    {
      if(root->child[i] != NULL){
        printAST(root->child[i], i, level+1);
      }
    }

    if(root->sibling != NULL)
    {
        printSiblingAST(root->sibling, ++siblingOrder, level); // if there is a sibling print it
    }
}

void printNode(AST_Tree* n)
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
            printf("Error printing node. NodeKind undefined\n");
            break;
    }
}

void printTerm(AST_Tree* n)
{
    switch(n->subkind.term)
    {
        case IDD:
            printf("Id: %s", n->attrib.name);
            break;

        default:
            printf("ERROR UNKNOWN TermKind\n");
            break;
    }
}

void printDecl(AST_Tree* n)
{
    switch(n->subkind.decl)
    {
        case FuncK:
            printf("Func: %s returns type %s", n->attrib.name, ExpTypeToStr(n->expType));
            break;

        case VarK:
            if(n->isArray) printf("Var: %s of array of type %s", n->attrib.name, ExpTypeToStr(n->expType));
            else if(n->isStatic) printf("Var: %s of static type %s", n->attrib.name, ExpTypeToStr(n->expType));
            else printf("Var: %s of type %s", n->attrib.name, ExpTypeToStr(n->expType));
            break;
        case ParamK:
            if(n->isArray) {printf("Parm: %s of array of type %s", n->attrib.name, ExpTypeToStr(n->expType));}
            else {printf("Parm: %s of type %s", n->attrib.name, ExpTypeToStr(n->expType));}
            break;
        default:
            printf("ERROR unknown subtype of DECL\n");
            break;
    }
}

void printStmt(AST_Tree* n)
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
            printf("ERROR UNKOWN StmtKind\n");
    }
}

void printExp(AST_Tree* n)
{
    switch(n->subkind.exp)
    {
        case ConstantK:
            printf("Const %s", n->attrib.name);
            break;
        case IdK:
            printf("Id: %s", n->attrib.name);
            break;
        case AssignK:
            printf("Assign: %s", n->attrib.name);
            break;
        case OpK:
            printf("Op: %s", n->attrib.name);
            break;
        case CallK:
            printf("Call: %s", n->attrib.name);
            break;
        case InitK:
            break;
        default:
            printf("Error printing subkind Exp, undefined\n");
            break;
    }
}
