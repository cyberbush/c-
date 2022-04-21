#ifndef EMITCODE_H_
#define EMITCODE_H_

using namespace std;

//
//  REGISTER DEFINES for optional use in calling the 
//  routines below.
//
#define GP   0	//  The global pointer
#define FP   1	//  The local frame pointer
#define RT   2	//  Return value
#define AC   3  //  Accumulator
#define AC1  4  //  Accumulator
#define AC2  5  //  Accumulator
#define AC3  6  //  Accumulator
#define PC   7	//  The program counter

//
//  No comment please...
//
#define NO_COMMENT (string )""


//
//  The following functions were borrowed from Tiny compiler code generator
//
int emitWhereAmI();           // gives where the next instruction will be placed
int emitSkip(int howMany);    // emitSkip(0) tells you where the next instruction will be placed
void emitNewLoc(int loc);     // set the instruction counter back to loc

void emitComment(string c);
void emitComment(string c, string cc);
void emitComment(string c, int n);

void emitGoto(int d, long long int s, string c);
void emitGoto(int d, long long int s, string c, string cc);
void emitGotoAbs(int a, string c);
void emitGotoAbs(int a, string c, string cc);

void emitRM(string op, long long int r, long long int d, long long int s, string c);
void emitRM(string op, long long int r, long long int d, long long int s, string c, string cc);
void emitRMAbs(string op, long long int r, long long int a, string c);
void emitRMAbs(string op, long long int r, long long int a, string c, string cc);

void emitRO(string op, long long int r, long long int s, long long int t, string c);
void emitRO(string op, long long int r, long long int s, long long int t, string c, string cc);

void backPatchAJumpToHere(int addr, string comment);
void backPatchAJumpToHere(string cmd, int reg, int addr, string comment);

int emitStrLit(int goffset, string s); // for char arrays

#endif
