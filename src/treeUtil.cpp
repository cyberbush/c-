//------------------ treeUtil.cpp ------------------
// Creates a new AST_Node node and set general type
#include "AST_Node.h"
#include "scanType.h"
#include "utils.h"
#include "treeUtil.h"

AST_Node* createNode(NodeKind nodeK)
{
    AST_Node* newNode = new AST_Node;
    newNode->sibling = NULL;
    newNode->child[0] = NULL;
    newNode->child[1] = NULL;
    newNode->child[2] = NULL;
    newNode->nodeKind = nodeK;
    return newNode;
}

// Takes a TokenData and creates a AST_Node node
static AST_Node* createNodeFromToken(TokenData* tokenData, int type)
{
    AST_Node* newNode = new AST_Node;
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
    newNode->printName = strdup(tokenData->tokenString);
    newNode->lineNum = tokenData->line;

    return newNode;
}


// Create ExpK (expression) node of type OpK (operator)
AST_Node* createOpNode(const char* s, int linenum, AST_Node* child0, AST_Node* child1, AST_Node* child2)
{
    AST_Node* newNode = new AST_Node;
    newNode->nodeKind = ExpK;
    newNode->subkind.exp = OpK;
    newNode->sibling = NULL;
    newNode->child[0] = child0;
    newNode->child[1] = child1;
    newNode->child[2] = child2;
    newNode->lineNum = linenum;

    if((s == "=") || (s == "><"))
      newNode->expType = Equal;

    // printf("Creating Op node: %s: on line number = %d\n", s, linenum);
    newNode->printName = s;

    return newNode;
}

// Create a StmtK node (statement)
AST_Node* createStmtNode(StmtKind stmtKind, const char* s, int linenum, AST_Node* child0, AST_Node* child1, AST_Node* child2)
{
    AST_Node* newNode = new AST_Node;
    newNode->nodeKind = StmtK;
    newNode->subkind.stmt = stmtKind;
    newNode->sibling = NULL;
    newNode->child[0] = child0;
    newNode->child[1] = child1;
    newNode->child[2] = child2;
    newNode->lineNum = linenum;
    newNode->printName = s;

    //printf("Creating Stmt node: %s: on line number = %d\n", s, linenum);

    return newNode;
}

// Create a DeclK node (declaration)
AST_Node* createDeclNode(DeclKind declKind, ExpType type, const char* s, int linenum, AST_Node* child0, AST_Node* child1, AST_Node* child2)
{
    AST_Node* newNode = new AST_Node;
    newNode->nodeKind = DeclK;
    if(declKind == ParamK) {newNode->varKind = Parameter;}
    newNode->subkind.decl = declKind;
    newNode->sibling = NULL;
    newNode->child[0] = child0;
    newNode->child[1] = child1;
    newNode->child[2] = child2;
    newNode->lineNum = linenum;
    newNode->printName = strdup(s);
    newNode->expType = type;

    // printf("Creating DeclK Node: %s: on line number = %d\n", s, linenum);

    return newNode;
}