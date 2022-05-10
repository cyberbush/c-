#include "codeGeneration.h"

// code generation

static void generateCode(AST_Node *n, int goffset, SymbolTable st)
{

    symbTable = st; // set symbol table

    goffsetFinal = goffset; // set the final goffset

    generateIO(); // add IO functions first before rest of code generation 

    generateNode(n, 0, true); // traverse tree and start generating code

    generateInit(); // Generate Program Initializer

}

static void generateNode(AST_Node *n, int offset, bool genSibling)
{
    if (n == NULL) return;

    int reset_offset, reset_offset2;

    switch(n->nodeKind) {

        case DeclK: // Declarations
            switch(n->subkind.decl) {
                case VarK:
                    switch(n->varKind) {
                        case Local:
                            if(n->hasInit) {
                                if(n->child[0]!=NULL && !(n->child[0]->expType==Char&&n->child[0]->varKind==Global&&n->child[0]->isArray==true)){ // not string
                                    generateNode(n->child[0], offset, true);
                                    emitRM("ST", 3, n->stackLocation, 1, "Store variable",string(n->name));
                                }
                                if(n->isArray) { // array
                                    emitRM("LDC", 3, n->size-1, 6, "load size of array", string(n->name));
                                    emitRM("ST", 3, n->stackLocation+1, 1, "save size of array", string(n->name));
                                }
                                if(isStr(n->child[0])) { // STRINGCONST
                                    if(isInStr(n->child[0]->name, '\\')) { // special chars
                                        if(isFirst) {
                                            isFirst = false;
                                        } 
                                        else if (!isFirst){
                                            goffsetFinal++;
                                            goffsetFinal += num_strs;
                                            n->child[0]->stackLocation++;
                                            n->child[0]->stackLocation+= num_strs;
                                            num_strs++;
                                        }
                                        n->child[0]->size--; 
                                        n->child[0]->name = createStrNoEscape(n->child[0]->name); 
                                    }
                                    
                                    generateNode(n->child[0], offset, true);
                                    emitRM("LDA", 3, n->child[0]->stackLocation, 0, "Load address of char array");
                                    emitRM("LDA", 4, n->stackLocation, 1, "address of lhs");
                                    emitRM("LD", 5, 1, 3, "size of rhs");
                                    emitRM("LD", 6, 1, 4, "size of lhs");
                                    emitRO("SWP", 5, 6, 6, "pick smallest size");
                                    emitRO("MOV", 4, 3, 5, "array op =");
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
                    emitComment("");
                    emitComment("** ** ** ** ** ** ** ** ** ** ** **");
                    emitComment("FUNCTION "+string(n->name));
                    emitComment("TOFF set: "+to_string(offset));
                    emitRM("ST", 3, -1, 1, "Store return address");
                    
                    generateNode(n->child[1], offset, true); // generate compound

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
                    generateNode(n->child[0], offset, true); // generate lhs
                    
                    if(n->child[1]!=NULL) { // binary op
                        emitRM("ST", 3, offset, 1, "Push left side"); // store lhs
                        emitComment("TOFF dec: "+to_string(--offset));
                        generateNode(n->child[1], offset, true); // generate rhs
                        emitComment("TOFF inc: "+to_string(++offset));
                        emitRM("LD", 4, offset, 1, "Pop left into ac1");
                        if(n->child[0]->isArray && n->child[1]->isArray) { emitOpArray(n, offset); }
                    }

                    emitOp(string(n->name)); // generates code based on operator

                    break;
                case ConstantK:
                    //emitComment("EXPRESSION");
                    switch(n->expType){
                        case Integer:
                            emitRM("LDC", 3, n->attrib.value, 6, "Load integer constant");
                            break;
                        case Char: // char and string constant
                            if(isStr(n)) { emitStrLit(n->stackLocation, string(n->name)); }
                            else { emitRM("LDC", 3, n->attrib.cvalue, 6, "Load char constant"); }
                            break;
                        case Boolean:
                            emitRM("LDC", 3, n->attrib.value, 6, "Load Boolean constant");
                            break;
                    }
                    break;
                case IdK:
                        if(n->varKind!=Global) { // local
                            if (n->isArray && n->varKind == Parameter) { emitRM("LD", 3, n->stackLocation, 1, "Load address of base of array", string(n->name)); }
                            else if(n->isArray) { emitRM("LDA", 3, n->stackLocation, n->varKind==Local, "Load address of base of array", string(n->name)); }
                            else { emitRM("LD", 3, n->stackLocation, n->varKind == Local || n->varKind == Parameter, "Load variable", string(n->name)); }
                        }
                        else { // global
                            if(n->isArray) { emitRM("LDA", 3, n->stackLocation, 0, "Load address of base of array", string(n->name)); }
                            else { emitRM("LD", 3, n->stackLocation, n->varKind == Local || n->varKind == Parameter, "Load variable", string(n->name)); }
                        }
                    break;
                case AssignK:
                {   string op = string(n->name);

                    if(isR(op)) { emitAssignOp(n, op, offset); break; } // generate assign op
                    
                    if(op == "--" || op == "++" ) { emitIncDecOp(n, op); break; } // generate inc/dec op
                    
                    if(!strcmp(n->child[0]->name,"[")) { emitAssignArray(n, n->child[0]->child[0], offset); } // generate assign array
                    else if(isStr(n->child[0])) {
                        generateNode(n->child[1], offset-1, true);
                        emitRM("LDA", 4, n->child[0]->stackLocation, 1, "address of lhs");
                        emitRM("LD", 5, 1, 3, "size of rhs");
                        emitRM("LD", 6, 1, 4, "size of lhs");
                        emitRO("SWP", 5, 6, 6, "pick smallest size");
                        emitRO("MOV", 4, 3, 5, "array op =");
                    }
                    else { // normal assign
                        generateNode(n->child[1], offset, true); // generate right child
                        emitRM("ST", 3, n->child[0]->stackLocation, n->child[0]->varKind == Local || n->child[0]->varKind == Parameter, "Store variable",string(n->child[0]->name));
                    }
                    
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
                    emitComment("IF");
                    generateNode(n->child[0], offset, true); // generate parameters 
                    reset_offset = emitSkip(1); // remember IF statement location, and leave space in instructions
                    emitComment("THEN");
                    generateNode(n->child[1], offset, true); // generate statement 
                    if(n->child[2] != NULL) { // check ELSE
                        reset_offset2 = emitSkip(1); // save location of THEN
                    }
                    backPatchAJumpToHere("JZR", 3, reset_offset, "Jump around the THEN if false [backpatch]");
                    if(n->child[2] != NULL) { // check for ELSE
                        emitComment("ELSE");
                        generateNode(n->child[2], offset, true);
                        backPatchAJumpToHere(reset_offset2, "Jump around the ELSE [backpatch]");
                    }
                    emitComment("END IF");
                    break;
                case WhileK:
                {
                    reset_offset = breakpoint; // rememember breakpoint
                    emitComment("WHILE");
                    int exp_loc = emitWhereAmI(); // save primary expression location
                    generateNode(n->child[0], offset, true); // generate primary expression
                    int do_loc = emitSkip(1); // save DO JMP location
                    breakpoint = emitSkip(1); // save BREAK JMP location
                    backPatchAJumpToHere("JNZ", 3, do_loc, "Jump to while part");
                    emitComment("DO");
                    generateNode(n->child[1], offset, true); // generate the statements
                    emitGotoAbs(exp_loc,"go to beginning of loop");
                    backPatchAJumpToHere(breakpoint, "Jump past loop [backpatch]");
                    emitComment("END WHILE");
                    breakpoint = reset_offset;
                }
                    break;
                case ForK:
                    //printASTAugmented(n, 0, 0);
                    emitComment("TOFF set: "+to_string(offset-3));
                    emitComment("FOR");
                    generateNode(n->child[0], offset, true); // generate variable code
                    generateNode(n->child[1], offset, true); // generate range code
                    reset_offset = emitSkip(1); // save location 
                    generateForComp(n->child[2], offset);
                    //generateNode(n->child[2], offset, true); // generate compound statement
                    emitComment("Bottom of loop increment and jump");
                    emitRM("LD", 3, offset, 1, "Load index");
                    emitRM("LD", 5, offset-2, 1, "Load step");
                    emitRO("ADD", 3, 3, 5, "increment");
                    emitRM("ST", 3, offset, 1, "store back to index");
                    emitGotoAbs(reset_offset-5, "go to beginning of loop");
                    backPatchAJumpToHere(reset_offset, "Jump past loop [backpatch]");
                    emitComment("END LOOP");
                    break;
                case RangeK:
                    //emitComment("RANGE");
                    generateNode(n->child[0], offset-3, true);
                    emitRM("ST", 3, offset, 1, "save starting value in index variable");
                    generateNode(n->child[1], offset-3, true);
                    emitRM("ST", 3, offset-1, 1, "save stop value");
                    if(n->child[2]!=NULL) { generateNode(n->child[2], offset-3, true); }
                    else { emitRM("LDC", 3, 1, 6, "default increment by 1"); }
                    emitRM("ST", 3, offset-2, 1, "save step value");
                    emitRM("LD", 4, offset, 1, "loop index");
                    emitRM("LD", 5, offset-1, 1, "stop value");
                    emitRM("LD", 3, offset-2, 1, "step value");
                    emitRO("SLT", 3, 4, 5, "Op <");
                    emitRM("JNZ", 3, 1, 7, "Jump to loop body");
                    break;
                case CompoundK:
                    reset_offset = offset;
                    offset = n->size;
                    emitComment("COMPOUND");
                    emitComment("TOFF set: "+to_string(offset));
                    generateNode(n->child[0], offset, true); // generate local declarations
                    emitComment("Compound Body");
                    generateNode(n->child[1], offset, true); // generate statement lists
                    offset = reset_offset;
                    emitComment("TOFF set: "+to_string(offset));
                    emitComment("END COMPOUND");
                    break;
                case ReturnK:
                        emitComment("RETURN");
                    if(n->expType!=Void) { 
                        generateNode(n->child[0], offset, true);
                        emitRM("LDA", 2, 0, 3, "Copy result to return register");
                    }
                    emitRM("LD", 3, -1, 1, "Load return address");
                    emitRM("LD", 1, 0, 1, "Adjust fp");
                    emitGoto(0, 3, "Return");                    
                    genSibling = false;
                    break;
                case BreakK:
                    emitComment("BREAK");
                    emitRM("JMP", 7, breakpoint-emitWhereAmI()-1, 7, "break");
                    break;
            }
            break;
    }

    if (genSibling) { generateNode(n->sibling, offset, true); } // generate code for sibling if set
}

// Generate the instructions for loading arguments from a function call
static void generateArgs(AST_Node *n, int toffset, int num_args)
{
    emitComment("TOFF dec: "+to_string(toffset));
    if(n==NULL) { return; }
    
    emitComment("Param "+to_string(num_args+1));
    
    generateNode(n, toffset, false); // generate code for the argument
    
    emitRM("ST", 3, toffset, 1, "Push parameter");
    
    generateArgs(n->sibling, toffset-1, ++num_args); // generate code for sibling arguments
}

//
static void generateForComp(AST_Node *n, int toffset)
{
    if(n!=NULL && n->nodeKind==StmtK && n->subkind.stmt==CompoundK) {
        toffset = n->size;
        int tmp = toffset;
        emitComment("COMPOUND");
        emitComment("TOFF set: "+to_string(toffset));
        generateNode(n->child[0], toffset, true); // generate local declarations
        emitComment("Compound Body");
        generateNode(n->child[1], toffset, true); // generate statement lists
        toffset = tmp;
        emitComment("TOFF set: "+to_string(toffset));
        emitComment("END COMPOUND");
    }
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
    else if(op == "!=") { emitRO("TNE", 3,4,3, "Op ><"); }
    else if(op == "*") { emitRO("MUL", 3, 4, 3, "Op *"); }
    else if(op == "+") { emitRO("ADD", 3, 4, 3, "Op +"); }
    else if(op == "-") { emitRO("SUB", 3, 4, 3, "Op -"); }
    else if(op == "/") { emitRO("DIV", 3, 4, 3, "Op /"); }
    else if(op == "%") { emitRO("MOD", 3, 4, 3, "Op %"); }
    else if(op == "[") {
        emitRO("SUB", 3, 4, 3, "compute location from index");
        emitRM("LD", 3, 0, 3, "Load array element");
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
    if(!strcmp(n->child[0]->name, "[")) { // array
        AST_Node *id = n->child[0]->child[0];
        generateNode(n->child[0]->child[1], toff-1, true);
        emitRM("ST", 3, toff, 1, "Push index");
        emitComment("TOFF dec: "+to_string(--toff));
        generateNode(n->child[1], toff, true);
        emitComment("TOFF inc: "+to_string(++toff));
        emitRM("LD", 4, toff, 1, "Pop index");
        if(id->varKind==Parameter) { emitRM("LD", 5, id->stackLocation, id->varKind == Local || id->varKind == Parameter, "Load address of base of array", string(id->name)); }
        else { emitRM("LDA", 5, id->stackLocation, id->varKind == Local || id->varKind == Parameter, "Load address of base of array", string(id->name)); }
        emitRO("SUB", 5, 5, 4, "Compute offset of value");
        emitRM("LD", 4, 0, 5, "load lhs variable",string(id->name));
        if (op == "+=") { emitRO("ADD", 3, 4, 3, "op +="); }
        else if(op == "-=") { emitRO("SUB", 3, 4, 3, "op -="); }
        else if(op == "*=") { emitRO("MUL", 3, 4, 3, "op *="); }
        else if(op == "/=") { emitRO("DIV", 3, 4, 3, "op /="); }
        emitRM("ST", 3, 0, 5, "Store variable", string(id->name));
    }
    else { // non-array
        generateNode(n->child[1], toff-1, true);
        emitRM("LD", 4, n->child[0]->stackLocation, n->child[0]->varKind == Local || n->child[0]->varKind == Parameter, "load lhs variable", string(n->child[0]->name)); //n->child[0]->varKind!=Global
        if (op == "+=") { emitRO("ADD", 3, 4, 3, "op +="); } 
        else if(op == "-=") { emitRO("SUB", 3, 4, 3, "op -="); }
        else if(op == "*=") { emitRO("MUL", 3, 4, 3, "op *="); }
        else if(op == "/=") { emitRO("DIV", 3, 4, 3, "op /="); }
        emitRM("ST", 3, n->child[0]->stackLocation, n->child[0]->varKind == Local || n->child[0]->varKind == Parameter, "Store variable", string(n->child[0]->name));
    }
}

static void emitIncDecOp(AST_Node *n, string op)
{
    if(!strcmp(n->child[0]->name, "[")) { // array
        AST_Node *id = n->child[0]->child[0];
        emitRM("LDC", 3, 3, 6, "Load integer constant");
        if(id->varKind==Parameter) { emitRM("LD", 5, id->stackLocation, id->varKind == Local || id->varKind == Parameter, "Load address of base of array", string(id->name)); }
        else { emitRM("LDA", 5, id->stackLocation, id->varKind == Local || id->varKind == Parameter, "Load address of base of array", string(id->name)); }
        emitRO("SUB", 5, 5, 3, "Compute offset of value");
        emitRM("LD", 3, 0, 5, "load lhs variable",string(id->name));
        if(op == "--") { emitRM("LDA", 3, -1, 3, "decrement value of",string(id->name)); }
        else if(op == "++") { emitRM("LDA", 3, 1, 3, "increment value of",string(id->name)); }
        emitRM("ST", 3, 0, 5, "Store variable", string(id->name));
    }  
    else { // not array
        emitRM("LD", 3, n->child[0]->stackLocation, n->child[0]->varKind == Local || n->child[0]->varKind == Parameter, "load lhs variable", string(n->child[0]->name));
        if(op == "--") { emitRM("LDA", 3, -1, 3, "decrement value of",string(n->child[0]->name)); }
        else if(op == "++") { emitRM("LDA", 3, 1, 3, "increment value of",string(n->child[0]->name)); }
        emitRM("ST", 3, n->child[0]->stackLocation, n->child[0]->varKind!=Global, "Store variable", string(n->child[0]->name));
    }
}

static void emitAssignArray(AST_Node *n, AST_Node* id, int offset)
{
    generateNode(n->child[0]->child[1], offset-1, true); // generate code rhs
    emitRM("ST", 3, offset, 1, "Push index");
    emitComment("TOFF dec: "+to_string(--offset));
    generateNode(n->child[1], offset, true); // generate code array
    emitComment("TOFF inc: "+to_string(++offset));
    emitRM("LD", 4, offset, 1, "Pop index");
    // param or non param?
    if(id->varKind==Parameter) { emitRM("LD", 5, id->stackLocation, 1, "Load address of base of array", string(id->name)); }
    else { emitRM("LDA", 5, id->stackLocation, id->varKind == Local || id->varKind == Parameter, "Load address of base of array", string(id->name)); }
    emitRO("SUB", 5, 5, 4, "Compute offset of value");
    emitRM("ST", 3, 0 ,5, "Store variable", string(id->name));
}

static void emitOpArray(AST_Node *n, int offset)
{
    emitRM("LD", 5, 1, 3, "AC2 <- |RHS|");
    emitRM("LD", 6, 1, 4, "AC3 <- |LHS|");
    emitRM("LDA", 2, 0, 5, "R2 <- |RHS|");
    emitRO("SWP", 5, 6, 6, "pick smallest size");
    emitRM("LD", 6, 1, 4, "AC3 <- |LHS|");
    emitRO("CO", 4, 3, 5, "setup array compare  LHS vs RHS");
    emitRO("TNE", 5, 4, 3, "if not equal then test (AC1, AC)");
    emitRO("JNZ", 5, 2, 7, "jump not equal");
    emitRM("LDA", 3, 0, 2, "AC1 <- |RHS|");
    emitRM("LDA", 4, 0, 6, "AC <- |LHS|");
}

// Generates initialization code that is called at the begining of the program.
static void generateInit()
{

    backPatchAJumpToHere(0, "Jump to init [backpatch]"); 

    emitComment("INIT");
    emitRM("LDA", 1, goffsetFinal, 0, "set first frame at end of globals");
    emitRM("ST", 1, 0, 1, "store old fp (point to self)");

    emitComment("INIT GLOBALS AND STATICS");
    symbTable.applyToAllGlobal(emitInitGlobals);
    //symbTable.print(printTest);
    emitComment("END INIT GLOBALS AND STATICS");

    emitRM("LDA", 3, 1, 7, "Return address in ac");
    emitGoto(mainLoc-emitWhereAmI()-1, 7, "Jump to main");
    emitRO("HALT", 0, 0, 0, "DONE!");
    emitComment("END INIT");
}

void printTest(void *data)
{
    AST_Node* n = (AST_Node*)data;
    printf("Line: %d\tName: %s\n", n->lineNum, n->name);
}

// Apply to Symbol table to Initialize globals and staticsS
void emitInitGlobals(string str, void* a)
{
    AST_Node* n = (AST_Node*)a; // get correct type
    if(n->subkind.decl == VarK &&  n->child[0] != NULL){
        generateNode(n->child[0], -2, true);
        emitRM("ST", 3, n->stackLocation, 0, "Store variable",string(n->name));
    }
    if(n->subkind.decl == VarK && n->isArray){
        emitRM("LDC", 3, n->size-1, 6, "load size of array",string(n->name));
        emitRM("ST", 3, n->stackLocation+1, 0, "save size of array",string(n->name));
    }
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
    // emitComment("C- compiler version C-S21");
    // emitComment("Built: Apr 18, 2021 (toffset telemetry)");
    // emitComment("Author: Robert B. Heckendorn");
    // emitComment("File compared: ");
    // emitComment("");
    // emitComment("** ** ** ** ** ** ** ** ** ** ** **");
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
}