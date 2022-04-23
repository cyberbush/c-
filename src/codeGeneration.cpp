#include "codeGeneration.h"

// code generation

static void generateCode(AST_Node *n, int goffset, SymbolTable st)
{

    symbTable = st; // set symbol table

    goffsetFinal = goffset; // set the final goffset

    generateIO(); // add IO functions first before rest of code generation 

    traverseTree(n, 0, true); // traverse tree and start generating code

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
                                    emitRM("ST", 3, n->stackLocation, 1, "Store variable",string(n->name));
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
                    emitComment("FUNCTION "+string(n->name));
                    emitComment("TOFF set: "+to_string(offset));
                    emitRM("ST", 3, -1, 1, "Store return address");
                    
                    traverseTree(n->child[1], offset, true); // generate compound

                    emitComment("Add standard closing in case there is no return statement");
                    emitRM("LDC", 2, 0 ,6 , "Set return value to 0");
                    
                    emitRM("LD", 3, -1, 1, "Load return address");
                    emitRM("LD", 1, 0, 1, "Adjust fp");
                    emitGoto(0, 3, "Return");
                    emitComment("END FUNCTION "+string(n->name));
                    break;
            }
            break;
        
        case ExpK: // Expressions
            switch(n->subkind.exp) {
                case OpK:
                    traverseTree(n->child[0], offset, true); // generate lhs

                    if(n->child[1]!=NULL) { // binary op
                        emitRM("ST", 3, offset, 1, "Push left side"); // store lhs
                        emitComment("TOFF dec: "+to_string(--offset));
                        traverseTree(n->child[1], offset, true); // generate rhs
                        emitComment("TOFF inc: "+to_string(++offset));
                        emitRM("LD", 4, offset, 1, "Pop left into ac1");
                    }

                    emitOp(string(n->name)); // generates code based on operator

                    break;
                case ConstantK:
                    emitComment("EXPRESSION");
                    switch(n->expType){
                        case Integer:
                            emitRM("LDC", 3, n->attrib.value, 6, "Load integer constant");
                            break;
                        case Char:
                            emitRM("LDC", 3, n->attrib.cvalue, 6, "Load char constant");
                            break;
                        case Boolean:
                            emitRM("LDC", 3, n->attrib.value, 6, "Load Boolean constant");
                            break;
                    }
                    break;
                case IdK:
                    switch(n->varKind) {
                        case Local:
                            emitRM("LD", 3, n->stackLocation, 1, "Load variable", string(n->name));
                            break;
                        case Global:
                            emitRM("LD", 3, n->stackLocation, 0, "Load variable", string(n->name));
                            break;
                        case LocalStatic:
                            emitRM("LD", 3, n->stackLocation, 0, "Load variable", string(n->name));
                            break;
                    }
                    break;
                case AssignK:
                {   string op = string(n->name);

                    if(isR(op)) { emitAssignOp(n, op, offset); break; } // generate assign op
                    
                    if(op == "--" || op == "++" ) { emitIncDecOp(n, op); break; } // generate inc/dec op
                    
                    traverseTree(n->child[1], offset, true); // generate right child
                    emitRM("ST", 3, n->child[0]->stackLocation, n->child[0]->varKind == Local || n->child[0]->varKind == Parameter, "Store variable",string(n->child[0]->name));
                    break;
                }
                case InitK:
                    break;
                case CallK:
                    reset_offset = offset;
                    emitComment("EXPRESSION");
                    emitComment("CALL "+string(n->name));
                    emitRM("ST", 1, offset, 1, "Store fp in ghost frame for",string(n->name));
                    emitComment("TOFF dec: "+to_string(--offset));

                    generateArgs(n->child[0], --offset, 0); // generate code for arguments

                    emitComment("Param end "+string(n->name));
                    offset = reset_offset;
                    emitRM("LDA", 1, offset, 1, "Ghost frame becomes new active frame");
                    emitRM("LDA", 3, 1, 7, "Return address in ac");
                    emitGoto(funcMap[n->name]-emitWhereAmI(), 7, "CALL",string(n->name));
                    emitRM("LDA", 3, 0, 2, "Save the result in ac");
                    emitComment("Call end "+string(n->name));
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
    emitComment("TOFF dec: "+to_string(toffset));
    if(n==NULL) { return; }
    
    emitComment("Param "+to_string(num_args+1));
    
    traverseTree(n, toffset, false); // generate code for the argument
    
    emitRM("ST", 3, toffset, 1, "Push parameter");
    
    generateArgs(n->sibling, toffset-1, num_args++); // generate code for sibling arguments
}

static void emitOp(string op)
{
    // binary
    if(op == "or") { emitRO("OR", 3, 4, 3, "Op OR"); }
    else if(op == "and") { emitRO("AND", 3, 4, 3, "Op AND"); }
    else if(op == "=") { emitRO("TEQ", 3, 4, 3, "Op ="); }
    else if(op == "<") { emitRO("TLT", 3, 4, 3, "Op <"); }
    else if(op == ">") { emitRO("TGT", 3, 4, 3, "Op >"); }
    else if(op == ">=") { emitRO("TGE", 3,4,3, "Op >="); }
    else if(op == "<=") { emitRO("TLE", 3,4,3, "Op <="); }
    else if(op == "><") { emitRO("TNE", 3,4,3, "Op ><"); }
    else if(op == "*") { emitRO("MUL", 3, 4, 3, "Op *"); }
    else if(op == "+") { emitRO("ADD", 3, 4, 3, "Op +"); }
    else if(op == "-") { emitRO("SUB", 3, 4, 3, "Op -"); }
    else if(op == "/") { emitRO("DIV", 3, 4, 3, "Op /"); }
    else if(op == "%") { emitRO("MOD", 3, 4, 3, "Op %"); }
    else if(op == "[") {
        emitRO("SUB", 3, 4, 5, "Compute offset of value");
        emitRM("LD", 3, 0, 3, "get value");
    }
    // unary
    else if(op == "not") {
        emitRM("LDC", 4, 1, 6, "Load 1");
        emitRO("XOR", 3, 3, 4, "Op XOR to get logical not");
    }
    else if(op == "chsign") { emitRO("NEG", 3, 3, 3, "Op unary -"); }
    else if(op == "sizeof") { emitRM("LD", 3, 1, 3, "Load array size"); }
    else if(op == "?")      { emitRO("RND", 3, 3, 6, "Op ?"); }
}

static void emitAssignOp(AST_Node *n, string op, int toff) 
{
    // array
    // traverseTree(n->child[0]->child[1], toff-1, true);
    // emitRM("ST", 3, toff, 1, "Push index");
    // emitComment("TOFF dec: "+to_string(--toff));
    // traverseTree(n->child[1], toff, true);
    // emitComment("TOFF inc: "+to_string(++toff));
    // emitRM("LD", 4, toff, 1, "Pop index");
    // non-array
    traverseTree(n->child[1], toff-1, true);
    emitRM("LD", 4, n->child[0]->stackLocation, n->child[0]->varKind!=Global, "load lhs variable", string(n->child[0]->name));
    if (op == "+=") { emitRO("ADD", 3, 4, 3, "op +="); }
    else if(op == "-=") { emitRO("SUB", 3, 4, 3, "op -="); }
    else if(op == "*=") { emitRO("MUL", 3, 4, 3, "op *="); }
    else if(op == "/=") { emitRO("DIV", 3, 4, 3, "op /="); }
    emitRM("ST", 3, n->child[0]->stackLocation, n->child[0]->varKind == Local || n->child[0]->varKind == Parameter, "Store variable", string(n->child[0]->name));
}

static void emitIncDecOp(AST_Node *n, string op)
{
    emitRM("LD", 3, n->child[0]->stackLocation, n->child[0]->varKind == Local || n->child[0]->varKind == Parameter, "load lhs variable", string(n->child[0]->name));
    if(op == "--") { emitRM("LDA", 3, -1, 3, "decrement value of",string(n->child[0]->name)); }
    else if(op == "++") { emitRM("LDA", 3, 1, 3, "increment value of",string(n->child[0]->name)); }
    emitRM("ST", 3, n->child[0]->stackLocation, n->child[0]->varKind!=Global, "Store variable", string(n->child[0]->name));
}

// Generates initialization code that is called at the begining of the program.
static void generateInit()
{

    backPatchAJumpToHere(0, "Jump to init [backpatch]"); 

    emitComment("INIT");
    emitRM("LDA", 1, goffsetFinal, 0, "set first frame at end of globals");
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
    //------------- header -------------
    emitComment("****************************************");
    emitComment("C- Compiler Version 1");
    emitComment("Built: Apr 19, 2022 (toffset telemetry");
    emitComment("Author: David C Bush");
    emitComment("");
    emitComment("** ** ** ** ** ** ** ** ** ** ** **");
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
    emitComment("");
    emitComment("** ** ** ** ** ** ** ** ** ** ** **");
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
    emitComment("");
    emitComment("** ** ** ** ** ** ** ** ** ** ** **");
    emitComment("FUNCTION inputb");
    emitRM("ST", 3, -1, 1, "Store return address");
    emitRO("INB", 2, 2, 2, "Grab bool input");
    emitRM("LD", 3, -1, 1, "Load return address");
    emitRM("LD", 1, 0, 1, "Adjust fp");
    emitGoto(0, 3, "Return");
    emitComment("END FUNCTION inputb");
    //------------- outputb -------------
    funcMap["outputb"] = emitWhereAmI()-1; 
    emitComment("");
    emitComment("** ** ** ** ** ** ** ** ** ** ** **");
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
    emitComment("");
    emitComment("** ** ** ** ** ** ** ** ** ** ** **");
    emitComment("FUNCTION inputc");
    emitRM("ST", 3, -1, 1, "Store return address");
    emitRO("INC", 2, 2, 2, "Grab char input");
    emitRM("LD", 3, -1, 1, "Load return address");
    emitRM("LD", 1, 0, 1, "Adjust fp");
    emitGoto(0, 3, "Return");
    emitComment("END FUNCTION inputc");
    //------------- outputc -------------
    funcMap["outputc"] = emitWhereAmI()-1; 
    emitComment("");
    emitComment("** ** ** ** ** ** ** ** ** ** ** **");
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
    emitComment("");
    emitComment("** ** ** ** ** ** ** ** ** ** ** **");
    emitComment("FUNCTION outnl");
    emitRM("ST", 3, -1, 1, "Store return address");
    emitRO("OUTNL", 3, 3, 3, "Output a newline");
    emitRM("LD", 3, -1, 1, "Load return address");
    emitRM("LD", 1, 0, 1, "Adjust fp");
    emitGoto(0, 3, "Return");
    emitComment("END FUNCTION outnl");
    emitComment("");
    emitComment("** ** ** ** ** ** ** ** ** ** ** **");
}