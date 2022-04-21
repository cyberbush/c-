#ifndef CODEGENERATOR_H_
#define CODEGENERATOR_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST_Node.h"
#include "utils.h"
#include "symbolTable.h"
#include "emitcode.cpp"

// Code generation

FILE *code; // file needed for emitcode
SymbolTable symbTable; // symbol table from semantic analysis
map<string, int> funcMap;

int pmem = 0;
int mainLoc = 0;
int mainRetLoc = 0;


static void generateCode(AST_Node *n, int goffset, SymbolTable st); // generates assembly code using the AST

static void traverseTree(AST_Node *n, int offset, bool genSibling); // traverses AST to generate code

static void generateIO(); // generates the code for the IO functions

static void generateInit(); // generates the code for Initialization thats called at begining of program

static void generateArgs(AST_Node *n, int toffset, int num_args); // Generate the instructions for loading arguments from a function call

#endif
