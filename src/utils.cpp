//------------------ utils.cpp ------------------
#include "utils.h"
/*
*   The utils file will be used for utility functions.
*/

void removeToken(TokenData** tok)
{
    if (tok == NULL && *tok == NULL) return;
    delete(*tok);
    *tok = NULL;
    return;
}
int countParams(AST_Node *t)
{
    int countP = 0;
    if(t->child[0] != NULL){
        AST_Node* l = t->child[0];
        while(l != NULL){
            t->params[countP] = {l->expType, l->isArray};
            countP++;
            l = l->sibling;
        }
    }
    return countP;
}

void printUsage()
{
    printf("usage: -c [options] [sourcefile]\n");
    printf("options:\n");
    printf("-d          - turn on parser debugging\n");
    printf("-D          - turn on symbol table debugging\n");
    printf("-h          - print this usage message\n");
    printf("-p          - print the abstract syntax tree\n");
    printf("-P          - print the abstract syntax tree plus type information\n");
}

bool isRL(string op)
{
    if(op == "and" || op == "or" || op == "<-" || op == "=" || op == "><" || op == "<" || op == "<=" || op == ">" || op == ">=" || op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        return true;
    }
    return false;
}
bool isR(string op)
{
    if(op == "+=" || op == "-=" || op == "*=" || op == "/=") {
        return true;
    }
    return false;
}
bool isL(string op)
{
    if(op == "[" || op == "sizeof" || op == "chsign" || op == "++" || op == "--" || op == "not") {
        return true;
    }
    return false;
}

bool isBoolExp(string op)
{
    if(op == "and" || op == "or" || op == "=" || op == "!=" || op == "<" || op == "<=" || op == ">" || op == ">=" || op == "not") {
        return true;
    }
    return false;
}

bool isInStr(const char *str, char c)
{
    return string(str).find(c) != string::npos;
}

int countSiblings(AST_Node* t)
{
    int count = 0;
    while (t != NULL)
    {
        count++;
        t = t->sibling;
    }
    return count;
}

AST_Node* getLastSibling(AST_Node* t)
{
    while ( t->sibling != NULL)
    {
        t = t->sibling;
    }
    return t;
}

const char* NodeKindToStr(NodeKind nk)
{
    switch(nk) {
        case DeclK:
            return "DeclK";
        case ExpK:
            return "ExpK";
        case StmtK:
            return "StmtK";
        case TermK:
            return "TermK";
        default:
            return "Error determining nodekind";
    }
}
// OpK, ConstantK, IdK, AssignK, InitK, CallK
const char* ExpKindToStr(ExpKind ek)
{
    switch(ek) {
        case OpK:
            return "OpK";
        case ConstantK:
            return "ConstantK";
        case IdK:
            return "IdK";
        case AssignK:
            return "AssignK";
        case InitK:
            return "InitK";
        case CallK:
            return "CallK";
        default:
            return "Error determining expkind";
    }
}

const char* DecltoStr(DeclKind dk)
{
    switch(dk){
        case VarK:
            return "VarK";
        case FuncK:
            return "FuncK";
        case ParamK:
            return "ParamK";
        default:
            return "Error unknown type";
    }
}

// print an expression type
const char* ExpTypeToStr(ExpType type)
{
    switch(type)
    {
        case Void:
            return "void";
            break;
        case Integer:
            return "int";
            break;
        case Boolean:
            return "bool";
            break;
        case Char:
            return "char";
            break;
        case CharInt:
            return "charInt";
            break;
        case Equal:
            return "Equal";
            break;
        case UndefinedType:
            return "Undefined Type";
            break;
        default:
            return "Error unknown type";
            break;
    }
}

// print out the string for the variable type
const char* VarKindToStr(VarKind vk)
{
    switch(vk)
    {
        case None:
            return "None";
            break;
        case Local:
            return "Local";
            break;
        case Global:
            return "Global";
            break;
        case Parameter:
            return "Parameter";
            break;
        case LocalStatic:
            return "LocalStatic";
            break;
        default:
            return "Error unknown variable type here\n";
            break;
    }
}

string to_string(AST_Node* n){
    string str = " [mem: "+string(VarKindToStr(n->varKind))+" loc: "+to_string(n->stackLocation)+" size: "+to_string(n->size)+"]";
    return str;
}

//
string createErr()
{
    return "ERROR(LINKER): A function named 'main' with no parameters must be defined.\n";
}
string createErr(string s1, int val)
{
    switch(val) {
        case 0:
            return "ERROR("+s1+"): Cannot return an array.\n";
        case 1:
            return "ERROR("+s1+"): Cannot have a break statement outside of loop.\n";
    }
    return "-11111111111111111";
}
string createErr(string s1, string s2, int val)
{
    switch(val){
        case 0:
            return "ERROR("+s1+"): The operation '"+s2+"' does not work with arrays.\n";
        case 1:
            return "ERROR("+s1+"): The operation '"+s2+"' only works with arrays.\n";
        case 2:
            return "ERROR("+s1+"): Array index is the unindexed array '"+s2+"'.\n";
        case 3:
            return "ERROR("+s1+"): Cannot index nonarray '"+s2+"'.\n";
        case 4:
            return "ERROR("+s1+"): Cannot use function '"+s2+"' as a variable.\n";
        case 5:
            return "ERROR("+s1+"): Symbol '"+s2+"' is not declared.\n";
        case 6:
            return "ERROR("+s1+"): '"+s2+"' is a simple variable and cannot be called.\n";
        case 7:
            return "ERROR("+s1+"): Cannot use array as test condition in "+s2+" statement.\n";
        case 8:
            return "ERROR("+s1+"): Cannot use array in position "+s2+" in range of for statement.\n";
        case 9:
            return "ERROR("+s1+"): Initializer for variable '"+s2+"' is not a constant expression.\n";
    }
    return "-11111111111111111";
}
string createErr(string s1, string s2, string s3, int val)
{
    switch(val){
        case 0:
            return "ERROR("+s1+"): Array '"+s2+"' should be indexed by type int but got type "+s3+".\n";
        case 1:
            return "ERROR("+s1+"): Symbol '"+s2+"' is already declared at line "+s3+".\n";
        case 2:
            return "ERROR("+s1+"): Function '"+s2+"' at line "+s3+" is expecting no return value, but return has a value.\n";
        case 3:
            return "ERROR("+s1+"): Expecting Boolean test condition in "+s2+" statement but got type "+s3+".\n";
        case 4: 
            return "ERROR("+s1+"): Too few parameters passed for function '"+s2+"' declared on line "+s3+".\n";
        case 5:
            return "ERROR("+s1+"): Too many parameters passed for function '"+s2+"' declared on line "+s3+".\n";
    }
    return "-111111111111111111";
}
string createErr(string s1, string s2, string s3, string s4, int val){
    switch(val){
        case 0:
            return "ERROR("+s1+"): '"+s2+"' requires operands of type "+s3+" but lhs is of type "+s4+".\n";
        case 1:
            return "ERROR("+s1+"): '"+s2+"' requires operands of type "+s3+" but rhs is of type "+s4+".\n";
        case 2:
            return "ERROR("+s1+"): '"+s2+"' requires operands of the same type but lhs is type "+s3+" and rhs is type "+s4+".\n";
        case 3:
            return "ERROR("+s1+"): Unary '"+s2+"' requires an operand of type "+s3+" but was given type "+s4+".\n";
        case 4:
            return "ERROR("+s1+"): '"+s2+"' requires both operands be arrays or not but lhs is"+s3+" an array and rhs is"+s4+" an array.\n";
        case 5: 
            return "ERROR("+s1+"): Function '"+s2+"' at line "+s3+" is expecting to return type "+s4+" but return has no value.\n";
        case 6: 
            return "ERROR("+s1+"): Expecting type "+s2+" in position "+s3+" in range of for statement but got type "+s4+".\n";
        case 7:
            return "ERROR("+s1+"): Expecting array in parameter "+s2+" of call to '"+s3+"' declared on line "+s4+".\n";
        case 8:
            return "ERROR("+s1+"): Initializer for variable '"+s2+"' of type "+s3+" is of type "+s4+"\n";
        case 9:
            return "ERROR("+s1+"): Initializer for variable '"+s2+"' requires both operands be arrays or not but variable is"+s3+" an array and rhs is"+s4+" an array.\n";
        case 10:
            return "ERROR("+s1+"): Not expecting array in parameter "+s2+" of call to '"+s3+"' declared on line "+s4+".\n";
    }
    return "-11111111111111111111";
}
string createErr(string s1, string s2, string s3, string s4, string s5, int val)
{   
    switch(val) {
        case 0:
            return "ERROR("+s1+"): Function '"+s2+"' at line "+s3+" is expecting to return type "+s4+" but returns type "+s5+".\n";
    }
    return "-11111111111111111111";
}
string createErr(string s1, string s2, string s3, string s4, string s5, string s6, int val)
{
    switch(val)
    {
        case 0:
            return "ERROR("+s1+"): Expecting type "+s2+" in parameter "+s3+" of call to '"+s4+"' declared on line "+s5+" but got type "+s6+".\n";
    }
    return "-11111111111111111111";
}


string createWarn(string s1, string s2, int val)
{
    switch(val){
        case 0:
            return "WARNING("+s1+"): Variable '"+s2+"' may be uninitialized when used here.\n";
        case 1:
            return "WARNING("+s1+"): The variable '"+s2+"' seems not to be used.\n";
        case 2:
            return "WARNING("+s1+"): The function '"+s2+"' seems not to be used.\n";
        case 3:
            return "WARNING("+s1+"): The parameter '"+s2+"' seems not to be used.\n";
    }
    return "-11111111111111111111";
}
string createWarn(string s1, string s2, string s3, int val) 
{
    switch(val) {
        case 0:
            return "WARNING("+s1+"): Expecting to return type "+s2+" but function '"+s3+"' has no return statement.\n";
    }
    return "-11111111111111111111";
}