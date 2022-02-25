//------------------ semantic.cpp ------------------
#include "semantic.h"
extern int errNum;
extern int warnNum;

SemanticAnalyzer::SemanticAnalyzer()
{
    isMain = false;
}

// start the semantic analysis
void SemanticAnalyzer::analyzeTree(AST_Node *root, bool symTableDebug)
{
    if(symTableDebug) symTable.debug(true); // set symbol table debugging

    firstTraversal(root); // first traversal through tree (preorder)

    manageUsedVars(symTable.getSymbols()); // check if there are any variables that arent used
    
    secondTraversal(root); // second traversal through tree (postorder)
    
    if(errNum > 0 || warnNum > 0) errors.printAll(); // print warnings and errors found
    
    if(!isMain) { errNum++; printf("ERROR(LINKER): A function named 'main()' must be defined.\n"); }
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
            manageUsedVars(symTable.getSymbols()); // check for used vars here
            symTable.leave();
        }        
    }
    else if (n->nodeKind == DeclK && n->subkind.decl == FuncK){ // check for function and leave scope
        // check for used vars here?
        manageUsedVars(symTable.getSymbols());
        symTable.leave();
    }
    if ( startFunction == n) { // check if we were in a function
        //if(startFunction->expType != Void && startFunction->hasReturn == false) { 
            // print warning since we're leaving non-void function without returning
            // warnNum++;
            //printf("Warning leaving non-void function without a return statement!\n");
        //}
        startFunction = NULL; // reset since we're leaving the function
    }
}

// Go through map of symbols and check if any of the variables haven't been used
void SemanticAnalyzer::manageUsedVars(map<string, void*> symbols){
    for(map<string, void*>::iterator it = symbols.begin(); it != symbols.end(); ++it){
        AST_Node* tmp = (AST_Node*)it->second;
        if(!tmp->isDeclUsed){ // check if its been used
            if(tmp->subkind.decl == VarK) { // check if its a variable
                // add warn: var seems to not be used
                errors.insertMsg(createWarn(to_string(tmp->lineNum), string(tmp->name),1) ,tmp->lineNum, 1);
            }
        }
    }
}

// Check to see if node is main and set isMain
void SemanticAnalyzer::checkMain(AST_Node *n)
{
    if(strcmp(n->name, "main") == 0) {
        if((n->expType == Void || n->expType == Integer) && n->num_params == 0) {
            isMain = true;
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
    if(n->isStatic) completed = symTable.insertGlobal(name, n);   
    else completed = symTable.insert(name, n);
    
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
            // check if break is inside loop??
            break;
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
    
    // check if returning an array
    AST_Node *child = n->child[0];
    if (child != NULL && child->isArray) { // add error: cant return array
        errors.insertMsg(createErr(to_string(n->lineNum)), n->lineNum, 0);
    }
    n->expType = startFunction->expType;
    n->name = strdup(startFunction->name);
    startFunction->hasReturn = true;
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
    if (tmp != NULL && !tmp->isInitialized && tmp->child[0] == NULL) { errors.insertMsg(createWarn(to_string(n->lineNum), string(n->name),0) ,n->lineNum, 1); }
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
        }
        else {
            copyNodeData(tmp, n);
            tmp->isDeclUsed;
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
            break;
        case WhileK:
            break;
        case ForK:
            break;
        case CompoundK:
            break;
        case ReturnK:
            // check if return is an array
            if(c != NULL && c->isArray) { errors.insertMsg(createErr(to_string(n->lineNum)),n->lineNum,0); }
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

// Handle any range errors that could occur
void SemanticAnalyzer::handleRangeErrors(AST_Node* n){
    int line = n->lineNum;
    AST_Node* c;
    for(int i = 0; i < 3; i++){
        c = n->child[i];
        if(c == NULL){
            continue; // no child exit early
        }
        else if(c->nodeKind == ExpK && (c->subkind.exp == IdK || c->subkind.exp == CallK) 
                && (AST_Node*)symTable.lookup(c->name) != NULL){
            continue; // exit because not declared
        }
        if(i == 0){
            // check for non-integer types
            if(c->subkind.exp == AssignK){
                if(!c->child[1]->isInitialized){ // add warn: var may be uninitialized
                    errors.insertMsg(createWarn(to_string(line), string(c->child[1]->name), 0),line,1);
                }
            }
            else if(c->subkind.exp == IdK && !c->isInitialized){ // add warn: var may be uninitialized
                errors.insertMsg(createWarn(to_string(line), string(c->name), 0),line,1);
            }
        }
        else if(i == 1 && c->expType != Integer){
            if(!c->isInitialized){ // add warn: var may be uninitialized
                errors.insertMsg(createWarn(to_string(line), string(c->name), 0),line,1);
            }
        }
        else if(c != NULL && c->expType != Integer){
            if(!c->isInitialized){ // add warn: var may be uninitialized
                errors.insertMsg(createWarn(to_string(line), string(c->name), 0),line,1);
            }
        }
    }
}
//__________________________________________________________________________________________
//__________________________________________________________________________________________
//___________________________________STOPPED HERE___________________________________________
//__________________________________________________________________________________________
//__________________________________________________________________________________________

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
            break; 
        default:
            printf("Error determing type in SemanticAnalyzer::handleExpErrors\n");
            break;
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
        if(op == "=") { outputT = Boolean; }
        else { outputT = compareBothNodeTypes(lhs, rhs, op, n->lineNum); }

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
        if(rhs->expType != Integer){ // check that rhs is int
            // add error: array should be indexed by type int
            errors.insertMsg(createErr(line, string(lhs->name), string(ExpTypeToStr(rhs->expType)), 0), n->lineNum,0);
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
    if(outputT != c->expType && op != "sizeof"){ // error: Unary operand types dont match
        errors.insertMsg(createErr(line, op, string(ExpTypeToStr(outputT)), string(ExpTypeToStr(c->expType)), 3),n->lineNum, 0);
    }
    if(op == "sizeof"){ // right side should be array
        if(!c->isArray){ // add error: operation only works with arrays
            errors.insertMsg(createErr(line, op, 1),n->lineNum, 0);
        }
    }
    else{
        if(c->isArray){ // add error: operation only does not work with arrays
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
    bool notVoid = (lhs->expType != Void && rhs->expType != Void);
    bool notUD = (lhs->expType != UndefinedType && rhs->expType != UndefinedType);

    if(lhs->expType != rhs->expType && notUD && notVoid){ // add error: operands not same type
        errors.insertMsg(createErr(to_string(line), op, string(ExpTypeToStr(lhs->expType)), string(ExpTypeToStr(rhs->expType)), 2), line, 0);
    }

    if((lhs->isArray || rhs->isArray) && (lhs->isArray != rhs->isArray)){ // add error: requires both operands be arrays
        string lhs_str = lhs->isArray ? "" : " not";
        string rhs_str = rhs->isArray ? "" : " not";
        errors.insertMsg(createErr(to_string(line), op, lhs_str, rhs_str, 4), line, 0);
    }
    return lhs->expType;
}