//------------------ MemoryTracker.h ------------------
#ifndef _MemoryTracker_H_
#define _MemoryTracker_H_

#include <iostream>
#include <vector>
#include "AST_Node.h"
using namespace std;

// Used to keep track of the offsets, locations, sizes for
// memory information. Used when traversing the tree in the semantic analysis
class MemoryTracker {
    private:
        int goffset; // global offset
        int foffset; // frame/local offset

        vector<pair<AST_Node*,int>> compStack; // stack to keep track of most recent compound
        int compCount = 0; // number of compound statements

        bool ignoreNextComp = false; // flag to ensure parameters are added to function scope

        void arrMem(AST_Node* n, int initSize);
        void addToCompSize(int size);

    public:
        MemoryTracker();            // constructor
        void varMem(AST_Node* n);   // update variable memory
        void paramMem(AST_Node* n); // update parameter memory
        void funcMem(AST_Node* n);  // update function memory
        void enteringComp(AST_Node* n);
        void leavingComp(AST_Node* n);
        void constMem(AST_Node* n); // update constant string memory
        void idMem(AST_Node* n, AST_Node* st_recent, AST_Node* st_all);
        void updateFoffset(int val);
        int getgoffset() { return goffset; }
};

#endif