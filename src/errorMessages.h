//------------------ errorMessages.h ------------------
#include <iostream>
#include <vector>
#include <map>
#include <string.h>
using namespace std;

// Class to manage warning and error messages found in the semantic analysis

class ErrorMessages {
    private:
    
        // map uses line number as key, and vector<string> to hold messages
        map<int, vector<string>> errMap;        // map of error messages
        map<int, vector<string>> warnMap;       // map of warning messages

        void printMsg(vector<string> msgs);

    public:

        void insertMsg(string msg, int line, int type);

        void printErr();
        void printWarn();
        void printAll();
};