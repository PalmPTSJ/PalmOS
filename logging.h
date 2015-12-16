#ifndef LOGGING_H_INCLUDED
#define LOGGING_H_INCLUDED

#define TIMESTAMP true

#include "stringUtil.h"

#include <string>
#include <stdio.h>
#include <sstream>
#include <map>
#include <stack>
#include <iostream>
using namespace std;

extern int timestamp;
string reqTimestamp();
string logHeader(string module,int leadingSpace = 0);
bool isLogEnable(string module);

class loggingClass {
public :
    loggingClass();
    ~loggingClass();

    void allowLog(string module);

    void startLog(string module);

    template<class t> loggingClass& operator<<(t targ) { /// this function can't splitted into cpp file
        logStream << targ;
        return *this;
    }
    loggingClass& operator<<(ostream& (*pf) (ostream&)) { /// endl; flush stream
        if(logStack.empty() || moduleAllowed.count(logStack.top()) != 0) cout << logStream.str() << endl;
        logStream.str("");
        logStream << logSpace;
        return *this;
    }

    void endLog();

    int logDepth;
    stack<string> logStack;
    map<string,bool> moduleAllowed;
    stringstream logStream;
    string logSpace;
};

/// interface for logging from networkHelper
void error(string str);
void debug(string str);
#endif // LOGGING_H_INCLUDED
