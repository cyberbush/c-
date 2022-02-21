//------------------ main.h ------------------
/*
    Main header file
*/
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "scanType.h"       // TokenData Type
#include "AST_Node.h"       // AST Tree Node Structure
#include "treeUtil.cpp"     // Tree utility functions
#include "utils.cpp"        // Utility functions
#include "printTree.cpp"    // Printing AST tree
#include "ourgetopt.cpp"    // Options handling
#include "semantic.cpp"     // Semantic analysis
#include "symbolTable.h"    // Symbol table