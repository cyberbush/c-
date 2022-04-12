//------------------ MemoryTracker.cpp ------------------

#include "MemoryTracker.h"

MemoryTracker::MemoryTracker()
{
    goffset = 0;
    foffset = 0;
}

// For handling variable memory.
// Can be array, global, or local memory
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

// For handling array memory
// Can be global or local
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
        foffset -= n->size;                 // update foffset for size
        addToCompSize(n->size);             // add to compound size since local scope
    }
}

// Increment the most recent compound statement size,
// used when adding variables
void MemoryTracker::addToCompSize(int size) 
{
     AST_Node* n = compStack.back().first; // most recent
     n->size += size;
}

// Handling parameter memory, 
// Can only be local and can only have size one
void MemoryTracker::paramMem(AST_Node* n)
{
    n->stackLocation = foffset--;
    n->size = 1;
}

// Handle function memory
void MemoryTracker::funcMem(AST_Node* n)
{
    foffset = -2;  // Reset the local frame pointer, always starts at -2 because of the old frame pointer and return address
    n->stackLocation = 0;   // functions start at location 0
    n->size = -1 * n->size; // funcSize = negative # of params 

    // add dummy to the stack to keep track of sizes
    AST_Node* tmp = new AST_Node;
    tmp->name = strdup("dummy");
    tmp->size = n->num_params;
    tmp->stackLocation = 0;
    compStack.push_back(std::make_pair(tmp, foffset));

    ignoreNextComp = true;
    compCount++;
}

// Entering new compound statement
void MemoryTracker::enteringComp(AST_Node* n) 
{
    // Make sure we dont add another scope if we just had a function
    if(ignoreNextComp){
        ignoreNextComp = !ignoreNextComp;
        return;
    }
    compCount++;
    // initialize location and size
    n->stackLocation = 0;
    n->size = 0; 
    compStack.push_back(std::make_pair(n, foffset)); // add to stack
}

// Leaving a compound
void MemoryTracker::leavingComp(AST_Node* n)
{
    compCount--;
    pair<AST_Node*, int> compPair = compStack.back(); // retrieve the most recent compound info
    compStack.pop_back(); // remove from top of stack
    n->size = (-1 * compPair.first->size) + compPair.second; // calculate size
    n->stackLocation = 0;
    foffset = compPair.second--; // reset
}

// handle constant memory
void MemoryTracker::constMem(AST_Node* n)
{
    if(n->stackLocation != 1) { return; }
    n->stackLocation = goffset - 1;
    goffset -= n->size;
}

// handle id memory
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