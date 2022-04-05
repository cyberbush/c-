//------------------ MemoryTracker.cpp ------------------

#include "MemoryTracker.h"

MemoryTracker::MemoryTracker() 
{
    goffset = 0;
    foffset = 0;
}

void MemoryTracker::varMem(AST_Node* n)
{
    // array offset
    if(n->isArray) {
        if(n->child[0] != NULL) {
            arrMem(n, n->child[0]->size);
        }
        else {
            arrMem(n, 0);
        }
        return;
    }

    // global offset
    if(n->varKind == LocalStatic || n->varKind == Global) {
        n->stackLocation = goffset; // assign location
        goffset -= n->size; // update goffset
        return;
    }
    // frame offset
    else {
        n->stackLocation = foffset;
        foffset -= n->size;
        addToCompSize(1); // add to the compound size since local
        return;
    }
}

void MemoryTracker::arrMem(AST_Node* n, int initSize) 
{
    if(initSize<0) { initSize = 0; }

    // global offset
    if(n->varKind == LocalStatic || n->varKind == Global) {
        if(initSize > 0){ n->child[0]->stackLocation = goffset - 1;} // place one below size location
        goffset -= initSize;                // move goffset down for initializer
        n->stackLocation = (goffset - 1);   // offset 1 to point below size
        goffset -= n->size;                 // update goffset for array
    }   
    // frame offset
    else {
        if(initSize > 0){                               // if there is an init, place it in next open goffset
            n->child[0]->stackLocation = goffset-1;     // move goffset down one afterwards
            goffset -= initSize;                        // offset by init size
        }
        n->stackLocation = foffset-1;       // offset 1 to point below size
        foffset -= n->size;                 // offset for size
        addToCompSize(n->size);             // add to compound size since local scope
    }
}

void MemoryTracker::addToCompSize(int size) 
{
     AST_Node* n = compStack.back().first; // most recent
     n->size += size;
}

void MemoryTracker::paramMem(AST_Node* n)
{
    n->stackLocation = foffset--; // parameters are local and can only be size 1
    n->size = 1;
}

void MemoryTracker::funcMem(AST_Node* n)
{
    foffset = -2; // reset local frame pointer to param start location
    n->stackLocation = 0;   // all functions appear to be declared at location 0
    n->size = -1 * n->size; // function size is -2, plus (negative)size of params

    // create a dummy to hold incremented sizes and things
    AST_Node* tmp = new AST_Node;
    tmp->name = strdup("dummy");
    tmp->size = n->num_params;
    tmp->stackLocation = 0;
    // add dummy and foffset to the stack of compound statements
    compStack.push_back(std::make_pair(tmp, foffset));

    // set compStmt ignore flag. since all funtions must be follwed by a compStmt, this will make sure parameters are entered into scope
    ignoreNextComp = true;
    compCount++;
}

void MemoryTracker::enteringComp(AST_Node* n) 
{
    // ignore_next_comp flags whether we have just entered a function, if so then the function scope is already created so don't create
    // a new scope for this compound
    if(ignoreNextComp){
        ignoreNextComp = !ignoreNextComp;
        return;
    }
    compCount++;
    // init memory/location
    n->stackLocation = 0;
    n->size = 0;    // initially use size to count memory used
    // save this compound on our stack of compounds so that we can track size
    compStack.push_back(std::make_pair(n, foffset)); // save foffset to reset to when reclaiming memory
}

void MemoryTracker::leavingComp(AST_Node* n)
{
    compCount--;
    pair<AST_Node*, int> compPair = compStack.back(); // get memory info from top of stack
    compStack.pop_back(); // remove node from stack
    n->size = (-1 * compPair.first->size) + compPair.second; // size is the original foffset + size of memory for variables in function
    n->stackLocation = 0;
    foffset = compPair.second--; // reset offset
}

void MemoryTracker::constMem(AST_Node* n)
{
    n->stackLocation = goffset-1;
    goffset -= n->size;
}

void MemoryTracker::idMem(AST_Node* n, AST_Node* st_recent, AST_Node* st_all)
{
    // copy information from the declaration
    if(st_recent != NULL){
        n->varKind = st_recent->varKind;
        n->stackLocation = st_recent->stackLocation;
        n->size = st_recent->size;
    }else if(st_all != NULL){
        n->varKind = st_all->varKind;
        n->stackLocation = st_all->stackLocation;
        n->size = st_all->size;
    }
}