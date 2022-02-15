//------------------ utils.cpp ------------------
#include "utils.h"
#include "AST_Node.h"
#include "scanType.h"
/*
*   The utils file will be used for utility functions.
*/

void removeToken(TokenData** tok)
{
    if (tok == NULL && *tok == NULL) return;
    delete(*tok);
    *tok = NULL;
    return;
}

void printUsage()
{
    printf("usage: -c [options] [sourcefile]\n");
    printf("options:\n");
    printf("-d          - turn on parser debugging\n");
    printf("-D          - turn on symbol table debugging\n");
    printf("-h          - print this usage message\n");
    printf("-p          - print the abstract syntax tree\n");
    printf("-P          - print the abstract syntax tree plus type information\n");
}

int countSiblings(AST_Node* t)
{
    int count = 0;
    while (t != NULL)
    {
        count++;
        t = t->sibling;
    }
    return count;
}

AST_Node* getLastSibling(AST_Node* t)
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
