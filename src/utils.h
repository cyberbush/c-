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

const char* NodeKindToStr(NodeKind nk);
const char* DecltoStr(DeclKind dk);
const char* ExpKindToStr(ExpKind ek);
// Takes an ExpType and returns the corresponding string
const char* ExpTypeToStr(ExpType type);
const char* VarKindToStr(VarKind vk);

string to_string(AST_Node* n);
void printMemory(AST_Node* n, bool newLine);

// check if operator needs to initialize rhs, lhs
bool isRL(string op);
bool isR(string op);
bool isL(string op);

bool isBoolExp(string op);
bool isInStr(const char* str, char c);

// functions for creating warnings and errors into strings
string createErr();
string createErr(string s1, int val);
string createErr(string s1, string s2, int val);
string createErr(string s1, string s2, string s3, int val);
string createErr(string s1, string s2, string s3, string s4, int val);
string createErr(string s1, string s2, string s3, string s4, string s5, int val);
string createErr(string s1, string s2, string s3, string s4, string s5, string s6, int val);

string createWarn(string s1, string s2, int val);
string createWarn(string s1, string s2, string s3, int val);

#endif
