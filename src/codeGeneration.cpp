#include "codeGeneration.h"

// code generation

static void generateCode(AST_Node *n, int goffset, SymbolTable st)
{
    code = fopen("testfile.tm", "a"); // file for emitcode

    symbTable = st; // set symbol table

    generateIO(); // add IO functions first before rest of code generation 

    traverseTree(n, goffset, true); // traverse tree and start generating code

    generateInit(); // Generate Program Initializer

}

static void traverseTree(AST_Node *n, int offset, bool genSibling)
{
    if (n == NULL) return;

    int reset_offset;

    switch(n->nodeKind) {

        case DeclK: // Declarations
            switch(n->subkind.decl) {
                case VarK:
                    switch(n->varKind) {
                        case Local:
                            if(n->hasInit) {
                                if(n->child[0] != NULL && !(n->child[0]->expType == Char && n->child[0]->size > 1)){ // not string
                                    traverseTree(n->child[0], offset, true);
                                    emitRM("ST", 3, n->stackLocation, 1, "Store variable "+string(n->name));
                                }
                            }
                            break;
                        case LocalStatic:
                            break;
                        case Global:
                            break;
                    }
                    break;
                case ParamK:
                    break;
                case FuncK:
                    funcMap[n->name] = emitWhereAmI()-1; // set functions start instruction
                    offset = -2 - n->num_params;
                    if(strcmp("main", n->name) == 0) mainLoc = emitWhereAmI();
                    emitComment("****************************************");
                    emitComment("FUNCTION "+string(n->name));
                    emitComment("TOFF set: "+to_string(offset));
                    emitRM("ST", 3, -1, 1, "Store return address");
                    
                    traverseTree(n->child[1], offset, true); // generate compound

                    emitComment("Add standard closing in case there is no return statement");
                    emitRM("LDC", 2, 0 ,6 , "Set return value 0");
                    
                    emitRM("LD", 3, -1, 1, "Load return address");
                    emitRM("LD", 1, 0, 1, "Adjust fp");
                    emitGoto(0, 3, "Return");
                    emitComment("END FUNCTION "+string(n->name));
                    if(strcmp("main", n->name)==0) { mainRetLoc=emitWhereAmI()-1; };
                    break;
            }
            break;
        
        case ExpK: // Expressions
            switch(n->subkind.exp) {
                case OpK:
                    break;
                case ConstantK:
                    switch(n->expType){
                        case Integer:
                            emitRM("LDC", 3, n->attrib.value, 6, "Load Integer constant");
                            break;
                        case Char:
                            emitRM("LDC", 3, n->attrib.cvalue, 6, "Load Char constant");
                            break;
                        case Boolean:
                            emitRM("LDC", 3, n->attrib.value, 6, "Load Boolean constant");
                            break;
                    }
                    break;
                case IdK:
                    emitRM("LD", 3, n->stackLocation, 1, "Load variable "+string(n->name));
                    break;
                case AssignK:
                    emitComment("EXPRESSION");
                    traverseTree(n->child[1], offset, true); // generate right child
                    traverseTree(n->child[0], offset, false); // generate id
                    break;
                case InitK:
                    break;
                case CallK:
                    reset_offset = offset;
                    emitComment("EXPRESSION");
                    emitComment("CALL "+string(n->name));
                    emitRM("ST", 1, offset, 1, "Store fp in ghost frame for output");
                    emitComment("TOFF dec: "+to_string(--offset));

                    generateArgs(n->child[0], --offset, 0); // generate code for arguments

                    emitComment("Param end "+string(n->name));
                    offset = reset_offset;
                    emitRM("LDA", 1, offset, 1, "Ghost frame becomes new active frame");
                    emitRM("LDA", 3, 1, 7, "Return address in ac");
                    emitGoto(funcMap[n->name]-emitWhereAmI(), 7, "Call "+string(n->name));
                    emitRM("LDA", 3, 0, 2, "Save the result ac");
                    emitComment("CALL end "+string(n->name));
                    emitComment("TOFF set: "+to_string(offset));
                    break;
            }
            break;
        
        case StmtK: // Statements
            switch(n->subkind.stmt) { // NullK, IfK, WhileK, ForK, CompoundK, ReturnK, BreakK, RangeK
                case NullK:
                    break;
                case IfK:
                    break;
                case WhileK:
                    break;
                case ForK:
                    break;
                case CompoundK:
                    reset_offset = offset;
                    offset = n->size;
                    emitComment("COMPOUND");
                    emitComment("TOFF set: "+to_string(offset));
                    emitComment("Compound Body");
                    
                    traverseTree(n->child[0], offset, true); // generate local declarations
                    traverseTree(n->child[1], offset, true); // generate statement lists
                    
                    offset = reset_offset;
                    emitComment("TOFF set: "+to_string(offset));
                    emitComment("END COMPOUND");

                    break;
                case ReturnK:
                    emitComment("RETURN");
                    emitRM("LD", 3, -1, 1, "Load return address");
                    emitRM("LD", 1, 0, 1, "Adjust fp");
                    emitGoto(0, 3, "Return");
                    break;
                case BreakK:
                    break;
                case RangeK:
                    break;
            }
            break;
    }

    if (genSibling) { traverseTree(n->sibling, offset, true); } // generate code for sibling if set
}

// Generate the instructions for loading arguments from a function call
static void generateArgs(AST_Node *n, int toffset, int num_args)
{
    emitComment("TOFF: dec: "+to_string(toffset));
    if(n==NULL) { return; }
    
    emitComment("Param "+to_string(num_args+1));
    
    traverseTree(n, toffset, false); // generate code for the argument
    
    emitRM("ST", 3, toffset, 1, "Push parameter");
    
    generateArgs(n->sibling, toffset-1, num_args++); // generate code for sibling arguments
}


// Generates initialization code that is called at the begining of the program.
static void generateInit()
{

    backPatchAJumpToHere(0, "Jump to init [backpatch]"); 

    emitComment("INIT");
    emitRM("LDA", 1, 0, 0, "set first frame at end of globals");
    emitRM("ST", 1, 0, 1, "store old fp (point to self)");

    emitComment("INIT GLOBALS AND STATICS");
    emitComment("END INIT GLOBALS AND STATICS");

    emitRM("LDA", 3, 1, 7, "Return address in ac");
    emitGoto(mainLoc-emitWhereAmI()-1, 7, "Jump to main");
    emitRO("HALT", 0, 0, 0, "DONE!");
    emitComment("END INIT");
}

// Generate the assembly code for all the IO functions
static void generateIO()
{
    emitSkip(1);
    pmem = 1; 
    //------------- header -------------
    emitComment("****************************************");
    emitComment("C- Compiler Version 1");
    emitComment("Built: Apr 19, 2022 (toffset telemetry");
    emitComment("Author: David C Bush");
    emitComment("****************************************");
    //------------- input -------------
    funcMap["input"] = emitWhereAmI()-1; // set functions start instruction
    emitComment("FUNCTION input");
    emitRM("ST", 3, -1, 1, "Store return address");
    emitRO("IN", 2, 2, 2, "Grab int input");
    emitRM("LD", 3, -1, 1, "Load return address");
    emitRM("LD", 1, 0, 1, "Adjust fp");
    emitGoto(0,3, "Return");
    emitComment("END FUNCTION input");
    //------------- output -------------
    funcMap["output"] = emitWhereAmI()-1; 
    emitComment("****************************************");
    emitComment("FUNCTION output");
    emitRM("ST", 3, -1, 1, "Store return address");
    emitRM("LD", 3, -2, 1, "Load parameter");
    emitRO("OUT", 3, 3, 3, "Output integer");
    emitRM("LD", 3, -1, 1, "Load return address");
    emitRM("LD", 1, 0, 1, "Adjust fp");
    emitGoto(0,3, "Return");
    emitComment("END FUNCTION output");
    //------------- inputb -------------
    funcMap["inputb"] = emitWhereAmI()-1; 
    emitComment("****************************************");
    emitComment("FUNCTION inputb");
    emitRM("ST", 3, -1, 1, "Store return address");
    emitRO("INB", 2, 2, 2, "Grab bool input");
    emitRM("LD", 3, -1, 1, "Load return address");
    emitRM("LD", 1, 0, 1, "Adjust fp");
    emitGoto(0, 3, "Return");
    emitComment("END FUNCTION inputb");
    //------------- outputb -------------
    funcMap["outputb"] = emitWhereAmI()-1; 
    emitComment("****************************************");
    emitComment("FUNCTION outputb");
    emitRM("ST", 3, -1, 1, "Store return address");
    emitRM("LD", 3, -2, 1, "Load parameter");
    emitRO("OUTB", 3, 3, 3, "Output bool");
    emitRM("LD", 3, -1, 1, "Load return address");
    emitRM("LD", 1, 0, 1, "Adjust fp");
    emitGoto(0, 3, "Return");
    emitComment("END FUNCTION outputb");
    //------------- inputc -------------
    funcMap["inputc"] = emitWhereAmI()-1; 
    emitComment("****************************************");
    emitComment("FUNCTION inputc");
    emitRM("ST", 3, -1, 1, "Store return address");
    emitRO("INC", 2, 2, 2, "Grab char input");
    emitRM("LD", 3, -1, 1, "Load return address");
    emitRM("LD", 1, 0, 1, "Adjust fp");
    emitGoto(0, 3, "Return");
    emitComment("END FUNCTION inputc");
    //------------- outputc -------------
    funcMap["outputc"] = emitWhereAmI()-1; 
    emitComment("****************************************");
    emitComment("FUNCTION outputc");
    emitRM("ST", 3, -1, 1, "Store return address");
    emitRM("LD", 3, -2, 1, "Load parameter");
    emitRO("OUTC", 3, 3, 3, "Output char");
    emitRM("LD", 3, -1, 1, "Load return address");
    emitRM("LD", 1, 0, 1, "Adjust fp");
    emitGoto(0, 3, "Return");
    emitComment("END FUNCTION outputc");
    //------------- outnl -------------
    funcMap["outnl"] = emitWhereAmI()-1; 
    emitComment("****************************************");
    emitComment("FUNCTION outnl");
    emitRM("ST", 3, -1, 1, "Store return address");
    emitRO("OUTNL", 3, 3, 3, "Output a newline");
    emitRM("LD", 3, -1, 1, "Load return address");
    emitRM("LD", 1, 0, 1, "Adjust fp");
    emitGoto(0, 3, "Return");
    emitComment("END FUNCTION outnl");
}