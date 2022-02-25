//------------------ utils.h ------------------
#ifndef _UTILS_H_
#define _UTILS_H_

#include "scanType.h"
#include "AST_Node.h"
#include "string.h"
using namespace std;

// Function to relinquish memory from token
static void removeToken(TokenData** tok);

// Function to print usage message
static void printUsage();

// Function to find the number of params for a function
int countParams(AST_Node *t);
// Function to count number of arguments for a AST Tree node
int countSiblings(AST_Node *t);
// Returns the sibling to the far right
AST_Node* getLastSibling(AST_Node *t);
// Takes an ExpType and returns the corresponding string
const char* ExpTypeToStr(ExpType type);
const char* VarKindToStr(VarKind vk);
string to_string(AST_Node* n);

// check if operator needs to initialize rhs, lhs
bool isRL(string op);
bool isR(string op);
bool isL(string op);

// functions for creating warnings and errors into strings
string createErr();
string createErr(string s1);
string createErr(string s1, string s2, int val);
string createErr(string s1, string s2, string s3, int val);
string createErr(string s1, string s2, string s3, string s4, int val);

string createWarn(string s1, string s2, int val);

#endif
