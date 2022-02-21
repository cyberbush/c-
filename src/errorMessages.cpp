#include "errorMessages.h"
extern int errNum;
extern int warnNum;

// add the error or warninng message as a string to our map using the line number as the key
void ErrorMessages::insertMsg(string msg, int line, int type)
{
    if ( type == 0) { // type 0 for error
        errMap[line].push_back(msg);
        errNum++;
    }
    else if (type == 1) { // type 1 for warning
        warnMap[line].push_back(msg);
        warnNum++;
    }
    else {
        printf("Error inserting message\n");
    }
}

// print only error messages
void ErrorMessages::printErr()
{
    map<int, vector<string>>::iterator it = errMap.begin();   
    for(it; it != errMap.end(); it++) {
        printMsg(it->second);
    }
}
// print only warning messages
void ErrorMessages::printWarn()
{
    map<int, vector<string>>::iterator it = warnMap.begin();   
    for(it; it != warnMap.end(); it++) {
        printMsg(it->second);
    }
}

// print the errors and warnings by line number
void ErrorMessages::printAll()
{
    map<int, vector<string>>::iterator it_err = errMap.begin();
    map<int, vector<string>>::iterator it_warn = warnMap.begin();
    while(it_err != errMap.end() || it_warn != warnMap.end()) 
    {
        if(it_err != errMap.end() && it_warn != warnMap.end()) { 
            if (it_err->first <= it_warn->first) { // error is lower number then warning
                // print error
                printMsg(it_err->second);
                it_err++;
            }
            else {
                //print warning
                printMsg(it_warn->second);
                it_warn++;
            }
        }
        else if(it_err != errMap.end()) { // check if there are more errors
            printMsg(it_err->second);
            it_err++;
        }
        else if(it_warn != warnMap.end()) { // check if there are more warnings
            // print warning
            printMsg(it_warn->second);
            it_warn++;
        }
    }
}

// print each vector of messages
void ErrorMessages::printMsg(vector<string> msgs)
{
    int size = msgs.size();
    for(int i=0; i<size; i++) {
        cout << msgs[i];
    }
}