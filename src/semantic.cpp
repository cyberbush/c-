//------------------ semantic.cpp ------------------
#include "semantic.h"
extern int errNum;
extern int warnNum;

SemanticAnalyzer::SemanticAnalyzer()
{
    // Build the Input/Output tree for semantic checking
    AST_Node* IO_root = buildIOTree();
    firstTraversal(IO_root);

    hasMain = false;
}

// build IO tree
AST_Node* SemanticAnalyzer::buildIOTree() 
{
    // build nodes
    AST_Node* input = createDeclNode(FuncK, Integer, "input", -1, NULL, NULL, NULL);
    input->isDeclUsed = true;
    input->hasReturn = true;
    AST_Node* inputb = createDeclNode(FuncK, Boolean, "inputb", -1, NULL, NULL, NULL);
    inputb->isDeclUsed = true;
    inputb->hasReturn = true;
    AST_Node* inputc = createDeclNode(FuncK, Char, "inputc", -1, NULL, NULL, NULL);
    inputc->isDeclUsed = true;
    inputc->hasReturn = true;
    AST_Node* output = createDeclNode(FuncK, Void, "output", -1, NULL, NULL, NULL);
    output->isDeclUsed = true;
    AST_Node* outputb = createDeclNode(FuncK, Void, "outputb", -1, NULL, NULL, NULL);
    outputb->isDeclUsed = true;
    AST_Node* outputc = createDeclNode(FuncK, Void, "outputc", -1, NULL, NULL, NULL);
    outputc->isDeclUsed = true;
    AST_Node* outnl = createDeclNode(FuncK, Void, "outnl", -1, NULL, NULL, NULL);
    outnl->isDeclUsed = true;

    // create sibling structure
    input->sibling = inputb;
    inputb->sibling = inputc;
    inputc->sibling = output;
    output->sibling = outputb;
    outputb->sibling = outputc;
    outputc->sibling = outnl;

    // create children
    AST_Node* dummy1 = createDeclNode(ParamK, Integer, "*dummy*", -1, NULL, NULL, NULL);
    dummy1->isDeclUsed = true;
    output->child[0] = dummy1;
    AST_Node* dummy2 = createDeclNode(ParamK, Boolean, "*dummy*", -1, NULL, NULL, NULL);
    dummy2->isDeclUsed = true;
    outputb->child[0] = dummy2;
    AST_Node* dummy3 = createDeclNode(ParamK, Char, "*dummy*", -1, NULL, NULL, NULL);
    dummy3->isDeclUsed = true;
    outputc->child[0] = dummy3;

    return input;
}

// start the semantic analysis
void SemanticAnalyzer::analyzeTree(AST_Node *root, bool symTableDebug)
{
    if(symTableDebug) symTable.debug(true); // set symbol table debugging

    firstTraversal(root); // first traversal through tree (preorder)

    manageUsedVars(symTable.getSymbols(), true); // check if there are any variables that arent used

    secondTraversal(root); // second traversal through tree (postorder)

    if(errNum > 0 || warnNum > 0) errors.printAll(); // print warnings and errors found
    
    if(!hasMain) { errNum++; printf("ERROR(LINKER): A function named 'main' with no parameters must be defined.\n"); }
}

// Traverse through each child and sibling for each node in tree (preorder)
void SemanticAnalyzer::firstTraversal(AST_Node *root)
{
    if(root==NULL) return;

    analyzeNode(root); // start analyzing the node

    // check child
    for(int i=0; i<3; i++) {
        if(root->child[i] != NULL) { firstTraversal(root->child[i]); }
    }
    
    manageScope(root); // manage the scope

    // check sibling
    if(root->sibling != NULL) { firstTraversal(root->sibling); }
}

// This function is used to manage the scope for the node its associated with
void SemanticAnalyzer::manageScope(AST_Node *n)
{
    if(n->nodeKind == StmtK) { // check for statement and leave scope
        StmtKind stmtKind = n->subkind.stmt;
        // check for for loop, while loop, compound
        if ((stmtKind == CompoundK || stmtKind == ForK || stmtKind == WhileK) && strcmp(n->name, "skip") != 0)  {
            manageUsedVars(symTable.getSymbols(), false); // check for used vars here
            symTable.leave();
        }        
    }
    else if (n->nodeKind == DeclK && n->subkind.decl == FuncK){ // check for function and leave scope
        // check for used vars here?
        manageUsedVars(symTable.getSymbols(), false);
        symTable.leave();
    }
    if ( startFunction == n) { // check if we were in a function
        if(startFunction->expType != Void && startFunction->hasReturn == false) { // print warn since we're leaving non-void function without returning
            // add warning: no return statement
            // MAYBE DONT NEED
            errors.insertMsg(createWarn(to_string(startFunction->lineNum), string(ExpTypeToStr(startFunction->expType)),string(startFunction->name), 0),startFunction->lineNum, 1);
        }
        startFunction = NULL; // reset since we're leaving the function
    }
}

// Go through map of symbols and check if any of the variables haven't been used
void SemanticAnalyzer::manageUsedVars(map<string, void*> symbols, bool end){
    for(map<string, void*>::iterator it = symbols.begin(); it != symbols.end(); ++it){
        AST_Node* tmp = (AST_Node*)it->second;
        if(!tmp->isDeclUsed){ // check if its been used
            if(tmp->subkind.decl == VarK) { // check if its a variable
                // add warn: var seems to not be used
                errors.insertMsg(createWarn(to_string(tmp->lineNum), string(tmp->name), 1) ,tmp->lineNum, 1);
            }
            else if(tmp->subkind.decl == ParamK) { // check if its a paramater
                // add warn: param seems to not be used
                errors.insertMsg(createWarn(to_string(tmp->lineNum), string(tmp->name), 3) ,tmp->lineNum, 1);
            }
            else if(end && !tmp->isMain && (strcmp(tmp->name, "main") != 0) && tmp->subkind.decl == FuncK) { // check if its a function
                // add warn: func seems to not be used
                errors.insertMsg(createWarn(to_string(tmp->lineNum), string(tmp->name), 2) ,tmp->lineNum, 1);
            }
        }
    }
}

// Check to see if node is main and set hasMain
void SemanticAnalyzer::checkMain(AST_Node *n)
{
    if(strcmp(n->name, "main") == 0) {
        if( (n->expType == Void || n->expType == Integer) && n->num_params == 0) {
            hasMain = true;
            n->isMain = true;
        }
    }
}

// Deal with node based on its nodeKind, Decl, Stmt, Exp
void SemanticAnalyzer::analyzeNode(AST_Node *n)
{
    switch(n->nodeKind) {
        case DeclK:
            analyzeDecl(n);
            break;
        case StmtK:
            analyzeStmt(n);
            break;
        case ExpK:
            analyzeExp(n);
            break;
        default:
            printf("Error in SemanticAnalyzer::analyzeNode1 determining type\n");
            break;
    }
}

// Function will analyze each type of declaration, VarK, ParamK, FuncK
void SemanticAnalyzer::analyzeDecl(AST_Node *n)
{
    switch(n->subkind.decl) {
        case VarK:
            handleVar(n);
            if(n->child[0] != NULL) { handleVarInit(n); }
            break;
        case ParamK:
            n->varKind = Parameter;     // set kind to Parameter
            n->isInitialized = true;    // parameters are automatically intitialized
            handleVar(n);
            break;
        case FuncK:
            handleFunc(n);
            break;
        default:
            printf("Error in Semanticanalyzezer::analyzeDecl determining type\n");
            break;
    }
}

// Inserts variable into symbol table and checks to see if its already been added
void SemanticAnalyzer::handleVar(AST_Node *n)
{
    string name = string(n->name); // has to be string for symbol table functions

    bool completed; // did the symbol table insert work?
    
    // try to insert into the symbol table
    // check for globabl scope?
    completed = symTable.insert(name, n);
    
    if (completed) { // inserted successfully
        AST_Node *global = (AST_Node*)symTable.lookupGlobal(name); // if not null then our variable is global
        AST_Node *local = (AST_Node*)symTable.lookup(name); // check to see if its local
        if (global != NULL) { // is global?
            n->varKind = Global;
            n->isInitialized = true;
        }
        else if (local != NULL && local != global) { // local or local static
            if(local->isStatic) n->varKind = LocalStatic;
            else n->varKind = Local;
        }
        if(n->child[0] != NULL) n->child[0]->varKind = Global;
    }
    else { // add error: symbol already added
        AST_Node *original = (AST_Node *)symTable.lookup(name);
        errors.insertMsg(createErr(to_string(n->lineNum), string(n->name), to_string(original->lineNum), 1), n->lineNum, 0);
    }
}

void SemanticAnalyzer::handleVarInit(AST_Node* n)
{
    if(n->isArray != n->child[0]->isArray){ // check if array and initializer is array
        string rhs = "";
        string lhs = "";
        if(n->isArray){ rhs = " not"; }
        else{ lhs = " not"; }
        // add error: intializer requires both operands be arrays
        errors.insertMsg(createErr(to_string(n->lineNum), string(n->name), lhs, rhs, 9), n->lineNum, 0);
    }
}

// Set the function as global, make a new scope, and insert into symbol table 
void SemanticAnalyzer::handleFunc(AST_Node *n)
{
    int paramCount = countParams(n); // returns number of paramaters
    n->num_params = paramCount; // store in node
    n->size = 2 + paramCount;   // set the size of the function

    checkMain(n); // check for main

    startFunction = n; // save reference to the start of the function

    // check number of parameters??

    // create new scope and check if its been declared already
    n->varKind = Global;
    symTable.enter(n->name);
    
    if(n->child[1] != NULL) n->child[1]->name = strdup("skip"); // dont make another scope for compound
    
    bool completed = symTable.insertGlobal(n->name, n);

    if (!completed) { // add error: function declared twice
        AST_Node *original = (AST_Node *)symTable.lookupGlobal(n->name); // retrieve original node
        errors.insertMsg(createErr(to_string(n->lineNum), string(n->name), to_string(original->lineNum), 1), n->lineNum, 0);
    }
}

// Check statements, enter scope for For, while, and compound statements
void SemanticAnalyzer::analyzeStmt(AST_Node *n)
{
    switch(n->subkind.stmt) {
        case NullK:
            break;
        case IfK:
            break;
        case WhileK:
            symTable.enter("While Loop");
            if(n->child[2] != NULL && n->child[2]->subkind.stmt == CompoundK) strcpy(n->child[2]->name, "skip");
            break;
        case ForK:
            symTable.enter("For Loop");
            if(n->child[2] != NULL && n->child[2]->subkind.stmt == CompoundK) strcpy(n->child[2]->name, "skip");
            break;
        case CompoundK:
            if(strcmp(n->name, "skip") != 0) { symTable.enter(n->name); }
            break;
        case ReturnK:
            if(startFunction != NULL) { handleReturn(n); }
            break;
        case BreakK:
        {
            // check if break is inside loop
            string scopeName = symTable.getScopeName();
            if(scopeName != "comp scope" && scopeName != "While Loop" && scopeName != "For Loop"){
                errors.insertMsg(createErr(to_string(n->lineNum), 1), n->lineNum, 0);
            }
            break;
        }
        case RangeK:
            break;
        default:
            printf("Error in SemanticAnalyzer::analyzeStmt determining type\n");
            break;
    }
    return;
}

// Deal with Return and check for errors
void SemanticAnalyzer::handleReturn(AST_Node *n)
{
    // Future:
    // check if void function returns non-void
    // check if non-void function returns void
    // check that return is same type as function
    //printf("line: %d\n", n->lineNum);
    // check return children are initialized

    if(startFunction != NULL) {
        string line = to_string(n->lineNum);
        string funLine = to_string(startFunction->lineNum);
        string name = string(startFunction->name);        
        AST_Node *tmp = NULL;
        if(n->child[0] != NULL) { tmp = (AST_Node*)symTable.lookup(n->child[0]->name); }

        if(startFunction->expType == Void && n->child[0] != NULL) { // check if void function returns
            errors.insertMsg(createErr(line, name, funLine, 2), n->lineNum, 0);
        }
        else if(n->child[0] == NULL && startFunction->expType != Void) { // check if non-void function has no return value
            errors.insertMsg(createErr(line, name, funLine, ExpTypeToStr(startFunction->expType), 5), n->lineNum, 0);
        }
        else if(n->child[0] != NULL && tmp != NULL && tmp->expType != startFunction->expType) { // check if return type matches
            errors.insertMsg(createErr(line, name, funLine, ExpTypeToStr(startFunction->expType), ExpTypeToStr(tmp->expType), 0), n->lineNum, 0);
        }
        else if(n->child[0] != NULL && n->child[0]->subkind.exp == ConstantK && n->child[0]->expType != startFunction->expType) { // constant doesnt match
            errors.insertMsg(createErr(line, name, funLine, ExpTypeToStr(startFunction->expType), ExpTypeToStr(n->child[0]->expType), 0), n->lineNum, 0);
        }

        // check if returning an array
        // AST_Node *child = n->child[0];
        // if (child != NULL && child->isArray) { // add error: cant return array
        //     errors.insertMsg(createErr(to_string(n->lineNum), 0), n->lineNum, 0);
        // }
        n->expType = startFunction->expType;
        n->name = strdup(startFunction->name);
        startFunction->hasReturn = true;
    }
}

// Check expressions
void SemanticAnalyzer::analyzeExp(AST_Node *n)
{
    switch(n->subkind.exp) {
        case OpK:
            checkOpChildInit(n->child[0], n->child[1], n->name);
            break;
        case ConstantK:
            break;
        case IdK:
            handleId(n);
            break;
        case AssignK:
            if(n->child[1] != NULL) checkVarSideInit(n->child[1]);
            initLeftVar(n);
            break;
        case InitK:
            handleInit(n);
            break;
        case CallK:
            handleCall(n);
            break;
        default:
            printf("Error in SemanticAnalyzer::analyzeExp determining type\n");
            break;
    }
    return;
}

// Check against using function as var, use of undeclared var
void SemanticAnalyzer::handleId(AST_Node *n)
{
    AST_Node *global = (AST_Node*)symTable.lookupGlobal(n->name);
    AST_Node *local = (AST_Node*)symTable.lookup(n->name);
    AST_Node* tmp = (AST_Node*)symTable.lookupGlobal(n->name);
 
    if(local != NULL) tmp = local;
    else tmp = global;

    if(tmp != NULL && tmp->subkind.decl == FuncK){ // add error: cant use funct as variable
            errors.insertMsg(createErr(to_string(n->lineNum), string(n->name), 4), n->lineNum, 0);
            tmp->isDeclUsed = n->isDeclUsed = true;
    }
    else if(tmp != NULL){ // if global exists, get type
            copyNodeData(tmp, n);
            tmp->isDeclUsed = n->isDeclUsed = true;

    }else{ // check most recent scope for previous declaration
        tmp = (AST_Node*)symTable.lookup(n->name);
        if(tmp == NULL){ // add error: symbol is not declared
            n->isInitialized = true; // if not declared, impossible to check if initialized
            errors.insertMsg(createErr(to_string(n->lineNum), string(n->name), 5), n->lineNum, 0);
            n->expType = UndefinedType;
            n->isArray = false;
            n->isStatic = false;
            n->isDeclUsed = true;
        }
        else{ // copy data to ID node
            copyNodeData(tmp, n);
            tmp->isDeclUsed = n->isDeclUsed = true;
            n->firstDecl = tmp;
        }
    }
    // check if ID is initialized, add warning
    if (tmp != NULL && !tmp->isInitialized && tmp->child[0] == NULL && tmp->nodeKind == DeclK && tmp->subkind.decl != FuncK) { 
        // add warn: var may be uninitialized
        errors.insertMsg(createWarn(to_string(n->lineNum), string(n->name),0) ,n->lineNum, 1);
        tmp->isInitialized = true; // stop cascading errors
    }
}

// Check call
void SemanticAnalyzer::handleCall(AST_Node *n)
{
    AST_Node *tmp = (AST_Node*)symTable.lookup(n->name);
    if(tmp != NULL) { // declared
        n->expType = tmp->expType;
        n->firstDecl = tmp;
        if(tmp->nodeKind == DeclK && tmp->subkind.decl != FuncK) { // check if its a func
            // add error: is a simple variable and cant be called
            errors.insertMsg(createErr(to_string(n->lineNum), string(n->name), 6), n->lineNum, 0);
            tmp->isDeclUsed = n->isDeclUsed = true;
        }
        else {
            copyNodeData(tmp, n);
            tmp->isDeclUsed = true;
        }
    }
    else { // add error: symbol is not declared
        errors.insertMsg(createErr(to_string(n->lineNum), string(n->name), 5), n->lineNum, 0);
    }

    // check all children to make sure they are initialized
    AST_Node *c = n->child[0];
    while(c != NULL) {
        checkVarSideInit(c);
        c = c->sibling;
    }

}

// Check init
void SemanticAnalyzer::handleInit(AST_Node *n)
{
    AST_Node *childL = n->child[0]; // ID on left side
    AST_Node *childR = n->child[1]; // other stuff on right side
    string lname = string(childL->name);
    
    // check to make sure right side is initialized
    checkOpChildInit(childL, childR, n->name);

    // is left expression
    if(lname == "[") {
        childL = childL->child[0];
        lname = childL->name;
    }

    // check if id is valid and set init
    AST_Node *tmp = (AST_Node*)symTable.lookup(lname);
    if (tmp != NULL) {
       tmp->isInitialized = true;
       childL->isInitialized = true;
    }
}

// check operation to make sure it's child are intialized
void SemanticAnalyzer::checkOpChildInit(AST_Node *lhs, AST_Node *rhs, string op)
{
    if(isL(op)) { // check if left associative operator
        checkVarSideInit(lhs);
    }
    else if(isR(op)) { // check if right associative
        checkVarSideInit(rhs);
    }
    else if (isRL(op)) { // check if both
        checkVarSideInit(lhs);
        checkVarSideInit(rhs);
    }
}

// check if variables are initialized for this side of the tree
void SemanticAnalyzer::checkVarSideInit(AST_Node *n)
{
    if(n->nodeKind == ExpK && n->subkind.exp == AssignK) initLeftVar(n);

    for(int i = 2; i >= 0; i--){
        if(n->child[i] != NULL) checkVarSideInit(n->child[i]);
    }

    if(n->nodeKind == ExpK && n->subkind.exp == IdK) checkVarInit(n);
}

// Check to make sure a variable is initialized
void SemanticAnalyzer::checkVarInit(AST_Node *n)
{
    if(strcmp(n->name, "[") == 0){ // is array?
        n = n->child[0]; // shift to find id for array
        n->isArray = true;
    }

    AST_Node *global = (AST_Node*)symTable.lookupGlobal(n->name);
    AST_Node *local = (AST_Node*)symTable.lookup(n->name);
    if(global != NULL && global->isArray == n->isArray){ // global array
        n->firstDecl = global;
        n->isInitialized = true;
        return; // already initialized by default
    }

    // local variable
    if(local != NULL && local->isStatic) n->isStatic = true;
    if(local != NULL && local->isInitialized){
        n->isInitialized = true;
        n->firstDecl = local;
    }
    
    // check conditions
    if(local != NULL && n->nodeKind == ExpK && !n->isInitialized && !n->isStatic){ 
        // add warn: variable may be uninitialized
        errors.insertMsg(createWarn(to_string(n->lineNum), string(n->name),0) ,n->lineNum, 1);
        n->isInitialized = true; // initialize so doesnt cause more errors
        if(local != NULL) local->isInitialized = true;
    }   
}

//  Iterates left until it finds IdK and then initializes it
void SemanticAnalyzer::initLeftVar(AST_Node *n)
{
    AST_Node* lhs = n->child[0];
    while(lhs != NULL && lhs->subkind.exp != IdK){
        lhs = lhs->child[0];
    }
    if(strcmp(n->name, "++") == 0 || strcmp(n->name, "--") == 0) {
        AST_Node* og = (AST_Node*)symTable.lookup(lhs->name);
        if(og != NULL && !og->isInitialized) {
            // add warn: variable may be uninitialized
            errors.insertMsg(createWarn(to_string(lhs->lineNum), string(lhs->name),0) ,lhs->lineNum, 1);
        }
    }
    lhs->isInitialized = true;
    AST_Node* tmp = (AST_Node*)symTable.lookup(lhs->name);
    // initialize the variable if its been declared
    if(tmp != NULL){
        tmp->isInitialized = true;
    }
}

// Goes through the tree for the second time using preorder
void SemanticAnalyzer::secondTraversal(AST_Node* root)
{
    if(root == NULL) return;

    // check children
    for(int i = 0; i < 3; i++)
    {
        if(root->child[i] != NULL) secondTraversal(root->child[i]);
    }

    // check sibling
    if(root->sibling != NULL) secondTraversal(root->sibling);

    analyzeNodeErrors(root); // check node
}

// Deal with node based on nodeKind
void SemanticAnalyzer::analyzeNodeErrors(AST_Node* n){
    switch(n->nodeKind){
        case DeclK: // dont need to deal with declarartions second time
            if(n->subkind.decl == VarK && n->child[0] != NULL) { handleInitErrors(n); } // check for init errors
            break;
        case StmtK:
            handleStmtErrors(n);
            break;
        case ExpK:
            handleExpErrors(n);
        default:
            break;
    }
}

// Here we handle the errors that statements may have
void SemanticAnalyzer::handleStmtErrors(AST_Node* n){
    AST_Node* c = n->child[0];
    switch(n->subkind.stmt) {
        case NullK:
            break;
        case IfK:
        {
            AST_Node *simpleExp = n->child[0];
            if(isNodeID_Array(simpleExp)){ // check if array and id
                errors.insertMsg(createErr(to_string(n->lineNum), "if", 7), n->lineNum, 0);
            }
            if(simpleExp->expType != Boolean && simpleExp->expType != UndefinedType && !isBoolExp(string(simpleExp->name))){ // check if type not boolean
                errors.insertMsg(createErr(to_string(n->lineNum), "if", string(ExpTypeToStr(simpleExp->expType)), 3), n->lineNum, 0);
            }
        }
            break;
        case WhileK:
        {
            AST_Node *simpleExp = n->child[0];
            if(isNodeID_Array(simpleExp)){ // check if array and id
                errors.insertMsg(createErr(to_string(n->lineNum), "while", 7), n->lineNum, 0);
            }
            if(simpleExp->expType != Boolean && simpleExp->expType != UndefinedType && !isBoolExp(string(simpleExp->name))){ // check if type not boolean
                errors.insertMsg(createErr(to_string(n->lineNum), "while", string(ExpTypeToStr(simpleExp->expType)), 3), n->lineNum, 0);
            }
        }
            break;
        case ForK:
            break;
        case CompoundK:
            break;
        case ReturnK:
            handleReturnInit(n);
            // check if return is an array
            if(c != NULL && c->isArray) { errors.insertMsg(createErr(to_string(n->lineNum), 0),n->lineNum,0); }
            break;
        case BreakK:
            break;
        case RangeK:
            handleRangeErrors(n);
            break;
        default:
            break;
    }
}

void SemanticAnalyzer::handleReturnInit(AST_Node *n)
{
    if(n == NULL) { return; }
    AST_Node* c1 = n->child[0];
    AST_Node* c2 = n->child[1];
    if(c1 != NULL) {
        if(c1->subkind.exp == IdK) { 
            AST_Node* global = (AST_Node*)symTable.lookupGlobal(c1->name);
            AST_Node* local = (AST_Node*)symTable.lookup(c1->name);
            AST_Node* tmp = global;
            if (local != NULL) { tmp = local; }
            else { tmp = global; }
            if (tmp != NULL && !tmp->isInitialized) {
                // add warn: variable may be uninitialized
                errors.insertMsg(createWarn(to_string(c1->lineNum), string(c1->name),0) ,c1->lineNum, 1);
                tmp->isInitialized = true; // stop cascading errors
            }
        }
        handleReturnInit(c1);
    }
    if(c2 != NULL) {
        if(c2->subkind.exp == IdK) { 
            AST_Node* global = (AST_Node*)symTable.lookupGlobal(c2->name);
            AST_Node* local = (AST_Node*)symTable.lookup(c2->name);
            AST_Node* tmp;
            if (local != NULL) { tmp = local; }
            else { tmp = global; }
 
            if (tmp != NULL && !tmp->isInitialized) {
                // add warn: variable may be uninitialized
                errors.insertMsg(createWarn(to_string(c2->lineNum), string(c2->name),0) ,c2->lineNum, 1);
                tmp->isInitialized = true; // stop cascading errors
            }
        }
        handleReturnInit(c2);
    }
}

// Handle any range errors that could occur
void SemanticAnalyzer::handleRangeErrors(AST_Node* n){

    handleFromToBy(n); // check the from x, to x, by x

    int line = n->lineNum;
    AST_Node* c;
    for(int i = 0; i < 3; i++){
        c = n->child[i];
        // if(c!=NULL && c->lineNum == 48) {
        //     printf("child: %d\tname: %s\t exp: %s\n", i, c->name, ExpTypeToStr(c->expType));
        // }
        if(c == NULL){
            continue; // no child exit early or undefined
        }
        else if(c->nodeKind == ExpK && (c->subkind.exp == IdK || c->subkind.exp == CallK) && (AST_Node*)symTable.lookup(c->name) != NULL) {
                // printf("exit early\n");
                continue; // exit because not declared
        }
        else if(c->expType == UndefinedType){
            continue;
        }
        if(i == 0) {
            // check for non-integer types
            if(c->subkind.exp == AssignK){
                if(!c->child[1]->isInitialized){ // add warn: var may be uninitialized
                    errors.insertMsg(createWarn(to_string(line), string(c->child[1]->name), 0),line,1);
                    c->child[1]->isInitialized = true; // stop cascading errors
                }
                if(c->child[1]->expType != Integer) { // check if integer
                    // add error: expecting int in position 1
                    errors.insertMsg(createErr(to_string(line), "int", to_string(i+1), string(ExpTypeToStr(c->expType)), 6), line, 0);
                }            
            }
            else if(c->expType != Integer) {
            // add error: expecting int in position 1
                errors.insertMsg(createErr(to_string(line), "int", to_string(i+1), string(ExpTypeToStr(c->expType)), 6), line, 0);
            }
            else if(c->subkind.exp == IdK && !c->isInitialized) {
                // add warn: variable may be uninitialized
                errors.insertMsg(createWarn(to_string(line), string(c->name), 0), line, 1);
                c->isInitialized = true; // stop cascading errors
            }
        
        }
        else if(i == 1 && c->expType != Integer){
            if(!c->isInitialized && !(c->nodeKind == ExpK && c->subkind.exp == ConstantK) ){ // add warn: var may be uninitialized 
                errors.insertMsg(createWarn(to_string(line), string(c->name), 0),line,1);
                c->isInitialized = true; // stop cascading errors
            }
            // add error: expecting int in position 2
            errors.insertMsg(createErr(to_string(line), "int", to_string(i+1), string(ExpTypeToStr(c->expType)), 6), line, 0);
        }
        else if(c != NULL && c->expType != Integer){
            // maybe check initialization
            // add error: expecting int in position i
            errors.insertMsg(createErr(to_string(line), "int", to_string(i+1), string(ExpTypeToStr(c->expType)), 6), line, 0);
        }
    }
}

// Function to check the from, to, and by of ranges
void SemanticAnalyzer::handleFromToBy(AST_Node *n)
{
    AST_Node *from = n->child[0]; // from x
    AST_Node *to = n->child[1]; // to x
    AST_Node *by = n->child[2]; // by x

    // check for arrays
    if(isNodeID_Array(from)){
        errors.insertMsg(createErr(to_string(n->lineNum),"1",8), n->lineNum, 0);
    }
    if(isNodeID_Array(to)){
        errors.insertMsg(createErr(to_string(n->lineNum),"2",8), n->lineNum, 0);
    }
    if(isNodeID_Array(by)){
        errors.insertMsg(createErr(to_string(n->lineNum),"3",8), n->lineNum, 0);
    }
}

// Handle any errors for expressions
void SemanticAnalyzer::handleExpErrors(AST_Node* n){

    // "expected" type that will be set to the node in this function
    ExpType exType = UndefinedType;
    int line = n->lineNum;
    string name = n->name;

    switch(n->subkind.stmt){
        case OpK:
            // check operation for errors
            exType = findAssOpType(n, OpK);
            n->expType = exType;
            break;
        case ConstantK:
            break;
        case IdK:
        {
            // set its type from the declaration
            AST_Node* tmp = (AST_Node*)symTable.lookup(n->name);
            if(tmp != NULL) exType = tmp->expType;
            break;
        }
        case InitK:
            break;
        case AssignK:
                // check types for assignops
                exType = findAssOpType(n, AssignK);
                n->expType = exType;
                n->isArray = n->child[0]->isArray;
                break;
        case CallK:
            handleCallErrors(n);
            break; 
        default:
            printf("Error determing type in SemanticAnalyzer::handleExpErrors\n");
            break;
    }
}

// check if leafs of initializer are constant
void SemanticAnalyzer::recurseInitializer(AST_Node *n) {
    if(n==NULL) { return; }
    if(n->child[0] != NULL) {
        recurseInitializer(n->child[0]);
    }
    if(n->child[1] != NULL) {
        recurseInitializer(n->child[1]);
    }
    if(n->child[0] == NULL && n->child[1] == NULL) { // leaf
        if(n->subkind.exp != ConstantK) { // if its not constant then initialzer isnt constant
            isInitConstant = false;
        }
        // printf("Leaf Node: %s, line: %d\n",n->name, n->lineNum);
    }
    // else {
    //     printf("Node: %s, line: %d\n",n->name, n->lineNum);
    // }
}

// Handle any InitK errors that could occur
void SemanticAnalyzer::handleInitErrors(AST_Node *n)
{
    isInitConstant = true;
    if(n->expType != n->child[0]->expType) { // types dont match
        // add error: initializer wrong type
        errors.insertMsg(createErr(to_string(n->lineNum), string(n->name), ExpTypeToStr(n->expType), ExpTypeToStr(n->child[0]->expType), 8), n->lineNum, 0);
    }
    else { // types matched
        if(n->child[0]->subkind.exp != ConstantK) { // check if constant
            recurseInitializer(n->child[0]); // check if leafs are constants
            if(!isInitConstant || strcmp(n->child[0]->name, "?") == 0) {
                // add error: initializer not constant
                errors.insertMsg(createErr(to_string(n->lineNum), string(n->name), 9), n->lineNum, 0);
            }
        }
    }
}

// Handle errors for calls
void SemanticAnalyzer::handleCallErrors(AST_Node *n)
{
   AST_Node *tmp = (AST_Node*)symTable.lookupGlobal(n->name);

    if(tmp != NULL && n->lineNum > tmp->lineNum){
        string line = to_string(n->lineNum);
        string name = string(n->name);
        string tmpLine = to_string(tmp->lineNum);
        // check if the number of params matches
        if(tmp->num_params > n->num_params){ // too few param
            errors.insertMsg(createErr(line, name, tmpLine, 4), n->lineNum, 0);

        }else if(tmp->num_params < n->num_params){ // too many param
            errors.insertMsg(createErr(line, name, tmpLine, 5), n->lineNum, 0);
        }
        
        // check param types
        AST_Node* l = n->child[0];
        for(int i = 0; i < tmp->num_params && l != NULL; i++){
            // check if types match and not undefined
            if(tmp->params[i].first != l->expType && tmp->params[i].first != UndefinedType && l->expType != UndefinedType){
                if(tmp->params[i].first == Boolean && isBoolExp(string(l->name))) {
                    // skip, the expected param is boolean and the child is a boolean operator
                    continue;
                }
                errors.insertMsg(createErr(line, string(ExpTypeToStr(tmp->params[i].first)), to_string(i+1), name, tmpLine, string(ExpTypeToStr(l->expType)), 0), n->lineNum, 0);
            }
            // check for array
            if(tmp->params[i].second != l->isArray){
                if(tmp->params[i].second){ // expecting array in param
                    errors.insertMsg(createErr(line, to_string(i+1), name, tmpLine, 7), n->lineNum, 0);
                }else{ // not expecting array
                    errors.insertMsg(createErr(line, to_string(i+1), name, tmpLine, 10), n->lineNum, 0);
                }
            }
            l = l->sibling;
        }
    }
}

// Find the expression type for operators and assignments 
ExpType SemanticAnalyzer::findAssOpType(AST_Node* n, ExpKind expK){
    ExpType outputType = UndefinedType;
    string name = n->name;

    // check if this is a constant expression
    bool isConstant = true;
    for(int i = 0; i < 3; i++){
      if(n->child[i] != NULL && n->child[i]->subkind.exp != ConstantK) isConstant = false;
    }
    
    // find type for children of assignment operator
    if(expK == AssignK){
        // unary or binary
        if(name == "++" || name == "--") outputType = findUnaryOp(n, name);
        else outputType = findBinaryOp(n, name);
    }
    else if(expK == OpK){ // find type for children of operators
        if(n->child[1] == NULL){ // unary
            outputType = findUnaryOp(n, name);
            if(strcmp("?", n->name) == 0) isConstant = false;
        }
        else{ outputType = findBinaryOp(n, name); } // binary
    }
    return outputType;
}

// do type checking for binary expressions
ExpType SemanticAnalyzer::findBinaryOp(AST_Node* n, string op){
    map<string, pair<ExpType, bool>> typeMap{
        {"+", {Integer, false}},       // lhs and rhs must be integers
        {"-", {Integer, false}},
        {"*", {Integer, false}},
        {"/", {Integer, false}},
        {"%", {Integer, false}},
        {"*=", {Integer, false}},
        {"/=", {Integer, false}},
        {"+=", {Integer, false}},
        {"-=", {Integer, false}},
        {"<", {Boolean, true}},       // requires booleans on both sides
        {">", {Boolean, true}},
        {"<=", {Boolean, true}},
        {">=", {Boolean, true}},
        {"!=", {Boolean, true}},
        {"or", {Boolean, false}},
        {"and", {Boolean, false}},
        {"=", {Equal, true}},         // only requires that both sides be same type
        {"<-", {Equal, true}},
        {"><", {Equal, true}},
        {"[", {UndefinedType, false}}, // returns the type associated with first child, requires int in brackets
        {":=", {UndefinedType, true}} // this one depends on the lhs which must match rhs
    };

    ExpType outputT = typeMap[op].first;    // the type expected for both sides
    bool acceptsArr = typeMap[op].second;   // if both sides can be arrays or not
    string name = string(n->name);
    string line = to_string(n->lineNum);
    // reference left and right sides
    AST_Node* lhs = n->child[0];
    AST_Node* rhs = n->child[1];

    // add error: operation does not work with arrays
    if((lhs->isArray || rhs->isArray) && !acceptsArr && op != "["){
        errors.insertMsg(createErr(line, op, 0),n->lineNum, 0);
    }

    if(outputT == Integer){
        compareBothTypes(lhs->expType, rhs->expType, outputT, op, n->lineNum);

    }else if(outputT == Boolean){
        if(op == "and" || op == "or") compareBothTypes(lhs->expType, rhs->expType, outputT, op, n->lineNum);
        else compareBothNodeTypes(lhs, rhs, op, n->lineNum);

    }else if(outputT == Equal){
        outputT = compareBothNodeTypes(lhs, rhs, op, n->lineNum);

    }else if(outputT == UndefinedType){
        outputT = lhs->expType; // type of id
        if(op == "["){
            if(!lhs->isArray){ // check that lhs is an array
                // add error: Cant index nonarray
                errors.insertMsg(createErr(line, string(lhs->name), 3), n->lineNum,0);
            }

            // find type
            bool isFine = true;
            AST_Node* l = rhs;
            if(l->child[0] != NULL && strcmp(l->name, "[") == 0){
                l = l->child[0];
            }
            if(l->nodeKind == ExpK && l->subkind.exp == IdK){
                isFine = l->firstDecl != NULL;
            }
            if(rhs->isArray){ // check that rhs is not an unindexed array
                // add error: array index is the unindexed array
                errors.insertMsg(createErr(line, string(rhs->name), 2), n->lineNum, 0);
                //errNum++;
            }
        }else if(op == "<-"){
            outputT = compareBothNodeTypes(lhs, rhs, op, n->lineNum);
        }
        if(rhs->expType != Integer && rhs->expType != UndefinedType){ // check that rhs is int
            AST_Node *tmp = (AST_Node*)symTable.lookupGlobal(rhs->name);
            if (tmp != NULL && tmp->subkind.decl != FuncK) { // make sure not function
                // add error: array should be indexed by type int
                errors.insertMsg(createErr(line, string(lhs->name), string(ExpTypeToStr(rhs->expType)), 0), n->lineNum,0);
            }
            else if(tmp != NULL && tmp->subkind.decl == FuncK && tmp->expType != Integer && tmp->expType != UndefinedType) {
                // add error: array should be indexed by type int
                errors.insertMsg(createErr(line, string(lhs->name), string(ExpTypeToStr(tmp->expType)), 0), n->lineNum,0);
            }
            else if (tmp == NULL) {
                // add error: array should be indexed by type int
               errors.insertMsg(createErr(line, string(lhs->name), string(ExpTypeToStr(rhs->expType)), 0), n->lineNum,0);
            }
        }
    }
    return outputT;
}

// do type checking for unary expressions and throw errors for bad types
ExpType SemanticAnalyzer::findUnaryOp(AST_Node* n, std::string op){
    map<string, ExpType> opsMap = {
        {"--", Integer},
        {"++", Integer},
        {"not", Boolean},
        {"sizeof", Integer},
        {"chsign", Integer},
        {"?", Integer}
    };
    ExpType outputT = opsMap[op];
    string line = to_string(n->lineNum);

    AST_Node* c = n->child[0];
    if(c->expType != UndefinedType && outputT != c->expType && op != "sizeof"){ // error: Unary operand types dont match
        errors.insertMsg(createErr(line, op, string(ExpTypeToStr(outputT)), string(ExpTypeToStr(c->expType)), 3),n->lineNum, 0);
    }
    if(op == "sizeof"){ // right side should be array
        if(!c->isArray && c->expType != UndefinedType){ // add error: operation only works with arrays
            errors.insertMsg(createErr(line, op, 1),n->lineNum, 0);
        }
    }
    else{
        if(c->isArray && c->expType != UndefinedType){ // add error: operation only does not work with arrays
            errors.insertMsg(createErr(line, op, 0),n->lineNum, 0);
        }
    }
    return outputT;
}

// check to see if each side is the expected type
void SemanticAnalyzer::compareBothTypes(ExpType lhs, ExpType rhs, ExpType expected, string op, int line){
    if(lhs != expected && lhs != UndefinedType){ // add error: unexpected type on lhs
        errors.insertMsg(createErr(to_string(line), op, string(ExpTypeToStr(expected)), string(ExpTypeToStr(lhs)), 0), line, 0);
    }
    if(rhs != expected && rhs != UndefinedType){ // add error: unexpected type on rhs
        errors.insertMsg(createErr(to_string(line), op, string(ExpTypeToStr(expected)), string(ExpTypeToStr(rhs)), 1), line, 0);
    }
}

// check that both sides are the same type and throw error if not
ExpType SemanticAnalyzer::compareBothNodeTypes(AST_Node* lhs, AST_Node* rhs, string op, int line){
    bool notUD = (lhs->expType != UndefinedType && rhs->expType != UndefinedType);

    if(lhs->expType != rhs->expType && notUD){ // add error: operands not same type
        errors.insertMsg(createErr(to_string(line), op, string(ExpTypeToStr(lhs->expType)), string(ExpTypeToStr(rhs->expType)), 2), line, 0);
    }

    if((lhs->isArray || rhs->isArray) && (lhs->isArray != rhs->isArray)){ // add error: requires both operands be arrays
        string lhs_str = lhs->isArray ? "" : " not";
        string rhs_str = rhs->isArray ? "" : " not";
        errors.insertMsg(createErr(to_string(line), op, lhs_str, rhs_str, 4), line, 0);
    }
    return lhs->expType;
}