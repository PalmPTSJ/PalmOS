#include "logging.h"

/**

startLog("OS");
log << "I'm logging" << endl
endLog("OS");

**/

string reqTimestamp() {
    if(TIMESTAMP) {
        string str = "";
        char timestampStr[100];
        sprintf(timestampStr,"{%06d}",timestamp);
        str.append(timestampStr);
        return str;
    }
    return "";
}
string logHeader(string module,int leadingSpace) {
    stringstream ss;
    for(int i = 0;i < leadingSpace;i++) ss << " ";
    ss << reqTimestamp() << " " << "[" << module << "]" << " ";
    return ss.str();
}
void error(string str) {
    cout << logHeader("ERROR") << str << endl;
}
void debug(string str) {
    cout << logHeader("SERVDBG") << str << endl;
}
bool isLogEnable(string module)
{
    /*if(module.compare("CONN")==0 && !LOG_CONN) return false;
    else if(module.compare("PARSE")==0 && !LOG_PARSE) return false;
    else if(module.compare("PACKET")==0 && !LOG_PACKET) return false;
    else if(module.compare("SERVDBG")==0 && !LOG_SERVDBG) return false;
    else if(module.compare("OS")==0 && !LOG_OS) return false;*/
    return true;
}

loggingClass::loggingClass() {
    logDepth = 0;
    logSpace = "";
}
loggingClass::~loggingClass() {

}
void loggingClass::allowLog(string module) {
    moduleAllowed.insert(make_pair(module,true));
}
void loggingClass::startLog(string module) {
    /// check if module is allowed
    if(module == "") {
        /// submodule
        logStack.push(logStack.top());
    }
    else {
        logStack.push(module);
        if(moduleAllowed.count(module) != 0) cout << logSpace << reqTimestamp() << " Logging module [" << module << "]"  << endl;
    }

    logDepth++;
    logSpace.push_back(' ');
    logSpace.push_back(' ');
    logStream.str("");
    logStream << logSpace;
}
void loggingClass::endLog() {
    if(logStack.empty()) {
        // error !
    }
    else {
        logStack.pop();
        logDepth--;
        logSpace.erase(logSpace.end()-1);
        logSpace.erase(logSpace.end()-1);
        logStream.str("");
        logStream << logSpace;
    }
}
