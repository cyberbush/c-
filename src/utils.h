//------------------ utils.h ------------------
#ifndef _UTILS_H_
#define _UTILS_H_

// Function to relinquish memory from token
static void removeToken(TokenData** tok);

// Function to count number of arguments for a AST Tree node
int countSiblings(AST_Node* t);
// Returns the sibling to the far right
AST_Node* getLastSibling(AST_Node* t);
// Takes an ExpType and returns the corresponding string
const char* ExpTypeToStr(ExpType type);

#endif
