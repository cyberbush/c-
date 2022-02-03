//------------------ utils.cpp ------------------
#include "utils.h"
#include "AST_Tree.h"
#include "scanType.h"
/*
*   The utils file will be used for utility functions
*   such as printing, building nodes, ect.
*/


// Creates a new AST_Tree node and set general type
AST_Tree* createNode(NodeKind nodeK)
{
    AST_Tree* newNode = new AST_Tree;
    newNode->sibling = NULL;
    newNode->child[0] = NULL;
    newNode->child[1] = NULL;
    newNode->child[2] = NULL;
    newNode->nodeKind = nodeK;
    return newNode;
}

// Takes a TokenData and creates a AST_Tree node
static AST_Tree* createNodeFromToken(TokenData* tokenData, int type)
{
    AST_Tree* newNode = new AST_Tree;
    newNode->sibling = NULL;
    newNode->child[0] = NULL;
    newNode->child[1] = NULL;
    newNode->child[2] = NULL;
    newNode->nodeKind = ExpK;
    switch(type) {
        case -1: // other ignore
            break;
        case 0: // Bool
            newNode->expType = Boolean;
            newNode->attrib.value = tokenData->nValue;
            break;
        case 1: // Char
            newNode->expType = Char; 
            newNode->attrib.cvalue = tokenData->cValue;
            break;
        case 2: // Int
            newNode->expType = Integer; 
            newNode->attrib.value = tokenData->nValue;
            break;
        case 3: // Assign
            newNode->subkind.exp = AssignK;
            break;
        case 4: // Operator
            newNode->subkind.exp = OpK;
            break;
        case 5: // ID
            newNode->subkind.exp = IdK;
            newNode->expType = UndefinedType;
            break;
        case 6: // Constant
            newNode->subkind.exp = ConstantK;
            break;
        default:
            printf("Error creating node from token, type: %d\n", type);
    }
    newNode->lineNum = tokenData->line;

    return newNode;
}


// Create ExpK (expression) node of type OpK (operator)
AST_Tree* createOpNode(const char* s, int linenum, AST_Tree* child0, AST_Tree* child1, AST_Tree* child2)
{
    AST_Tree* newNode = new AST_Tree;
    newNode->nodeKind = ExpK;
    newNode->subkind.exp = OpK;
    newNode->sibling = NULL;
    newNode->child[0] = child0;
    newNode->child[1] = child1;
    newNode->child[2] = child2;
    newNode->lineNum = linenum;

    if((s == "=") || (s == "><"))
      newNode->expType = Equal;

    // printf("CREATING NODE FOR OP %s: on line number = %d\n", s, linenum);
    newNode->attrib.name = s;

    return newNode;
}

// Create a StmtK node (statement)
AST_Tree* createStmtNode(StmtKind stmtKind, const char* s, int linenum, AST_Tree* child0, AST_Tree* child1, AST_Tree* child2)
{
    AST_Tree* newNode = new AST_Tree;
    newNode->nodeKind = StmtK;
    newNode->subkind.stmt = stmtKind;
    newNode->sibling = NULL;
    newNode->child[0] = child0;
    newNode->child[1] = child1;
    newNode->child[2] = child2;
    newNode->lineNum = linenum;
    newNode->attrib.name = s;

    // printf("CREATING NODE FOR STMT %s: on line number = %d\n", s, linenum);

    return newNode;
}

// Create a DeclK node (declaration)
AST_Tree* createDeclNode(DeclKind declKind, ExpType type, const char* s, int linenum, AST_Tree* child0, AST_Tree* child1, AST_Tree* child2)
{
    AST_Tree* newNode = new AST_Tree;
    newNode->nodeKind = DeclK;
    if(declKind == ParamK) {newNode->varKind = Parameter;}
    newNode->subkind.decl = declKind;
    newNode->sibling = NULL;
    newNode->child[0] = child0;
    newNode->child[1] = child1;
    newNode->child[2] = child2;
    newNode->lineNum = linenum;
    newNode->attrib.name = s;
    newNode->expType = type;

    // printf("CREATING NODE FOR DECL %s: on line number = %d\n", s, linenum);

    return newNode;
}

int countSiblings(AST_Tree* t)
{
    int count = 0;
    while (t != NULL)
    {
        count++;
        t = t->sibling;
    }
    return count;
}

AST_Tree* getLastSibling(AST_Tree* t)
{
    while ( t->sibling != NULL)
    {
        t = t->sibling;
    }
    return t;
}
// print an expression type
const char* ExpTypeToStr(ExpType type)
{
    switch(type)
    {
        case Void:
            return "void";
            break;
        case Integer:
            return "int";
            break;
        case Boolean:
            return "bool";
            break;
        case Char:
            return "char";
            break;
        case CharInt:
            return "charInt";
            break;
        case Equal:
            return "Equal";
            break;
        case UndefinedType:
            return "Undefined Type";
            break;
        default:
            return "Error unknown type";
            break;
    }
}
