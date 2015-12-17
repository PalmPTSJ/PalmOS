#define WINVER 0x0601
#include "network_core.h"
#include "network_server.h"
#include "network_client.h"
#include "stringUtil.h"
#include "logging.h"
#include <bits/stdc++.h>
#include <windows.h>
#include <Psapi.h>

#define WS_OP_TXT 129 // WebSocket Opcode "Text" data
#define WS_OP_BIN 130 // WebSocket Opcode "Binary" data

#define LOG_CONN TRUE // Connection ( Websocket handshake , id assignment )
#define LOG_SERVDBG TRUE // Server ( Connect , Accept , Disconnect )
#define LOG_PARSE FALSE // Parse ( WebSocket packet parsing )
#define LOG_PACKET TRUE // Packet ( Arrival , Info )
#define LOG_OS TRUE // OS ( OS command )
#define LOG_HTTP FALSE // HTTP ( HTTP request )

#define PRIV_FILEACCESS 1 // RETR RETD
#define PRIV_SOCKET 2 // Socket operations
#define PRIV_EXEC 4 // Execute file , system()
#define PRIV_INPUT 8 // Input mouse & Keyboard
#define PRIV_WINSTAT 16 // getWindowList , setFocus ...

/// DEFINITION
net_server_serverClass server;
struct clientS {
    byteArray recvBuffer;
    byteArray waitingBuffer;
    string ip;
    wstring appname;
    int socket_idRunner;
    map<int,net_client_clientClass> socketList;
};
struct packetS {
    char enc;
    string opcode;
    string id;
    wstring data;
    string trimData;
    int type;
};

int timestamp;
loggingClass logger;


typedef string (*modf_who)();
typedef string (*modf_version)();
typedef packetS (*modf_exec)(packetS);
typedef void (*modf_poll)();
typedef vector<string> (*modf_load)();
typedef void (*modf_unload)();

typedef void (*modflink_sendTo)(string,int);

typedef void (*modf_link)(modflink_sendTo); /// link function from here to module

struct moduleS {
    HINSTANCE hin;
    vector<string> moduleList;
    vector<string> flagList;
    int pollTime;
    int pollDelay;
    string who;
    string version;
    modf_exec func_exec;
    modf_poll func_poll;
    modf_unload func_unload;
};

vector<clientS> clientList;
vector<moduleS> moduleList;

string workingDir;
map<string,vector<int> > clientFlag;
map<string,string> contentTypeMap;
map<string,moduleS*> moduleMap;
vector<INPUT> inputQueue;
/// STATIC VAR , ASSIGNMENT
void initContentTypeMap()
{
    // Text
    contentTypeMap[".html"] = "text/html";
    contentTypeMap[".js"] = "application/javascript";
    contentTypeMap[".css"] = "text/css";
    contentTypeMap[".txt"] = "text/plain";
    // Image
    contentTypeMap[".bmp"] = "image/bmp";
    contentTypeMap[".gif"] = "image/gif";
    contentTypeMap[".jpg"] = "image/jpeg";
    contentTypeMap[".jpeg"] = "image/jpeg";
    contentTypeMap[".png"] = "image/png";
    // Audio
    contentTypeMap[".mp3"] = "audio/mpeg";
    contentTypeMap[".ogg"] = "audio/ogg";
    contentTypeMap[".wav"] = "audio/vnd.wave";
    // Video
    contentTypeMap[".mp4"] = "video/mp4";
    contentTypeMap[".avi"] = "video/avi";
    contentTypeMap[".wmv"] = "video/x-ms-wmv";
}
/// WebSocket FUNCTION
string wsHandshake(string str)
{
    /* HTTP/1.1 101 Switching Protocols
    Upgrade: websocket
    Connection: Upgrade
    Sec-WebSocket-Accept: HSmrc0sMlYUkAGmm5OPpG2HaGWk=
    (Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==)
    */
    string key;
    int keyInd = str.find("Sec-WebSocket-Key: ");
    key = str.substr(keyInd+19,24);
    string GUID_str = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    key.append(GUID_str);
    string sha1 = SHA1(key);
    string accept = base64(sha1);
    string toRet = "";
    toRet.append("HTTP/1.1 101 Switching Protocols\r\n");
    toRet.append("Upgrade: websocket\r\n");
    toRet.append("Connection: Upgrade\r\n");
    toRet.append("Sec-WebSocket-Accept: "); toRet.append(accept); toRet.append("\r\n");
    toRet.append("\r\n");
    return toRet;
}
byteArray wsPingMsg()
{
    byteArray toRet;
    toRet.push_back(137);
    toRet.push_back(0);
    toRet.push_back(0);
    return toRet;
}
bool wsDecodeMsg(clientS& clientData,packetS& pData)
{
    logger << "Parsing a packet" << endl;
    pData.data = L"";
    pData.opcode = pData.trimData = pData.id = "";
    int meta[6];
    meta[0] = clientData.recvBuffer[0]; // type & everything
    meta[1] = clientData.recvBuffer[1] & 127; // length + masked
    int nowPtr = 2;
    int metaLen = 6;
    unsigned long long sz = 0;
    if(meta[1] == 126) { // additional 2 bytes will be used for length
        for(int i = 2;i < 4;i++) {sz<<=8; sz+=clientData.recvBuffer[i];}
        nowPtr = 4;
        metaLen = 8;
    }
    else if(meta[1] == 127) { // additional 8 bytes will be used for length
        for(int i = 2;i < 10;i++) {sz<<=8; sz+=clientData.recvBuffer[i];}
        nowPtr = 10;
        metaLen = 14;
    }
    else {
        sz = meta[1];
    }
    for(int i = 0;i < 4;i++) {
        meta[i+2] = clientData.recvBuffer[nowPtr++];
    }
    // split message with sz
    if((clientData.recvBuffer.size()-metaLen)<sz) {
        logger << "BufferSize - metaLen < sz , data might be incompleted" << endl;
        return false;
    }
    int charcode = 0;
    int packetFIN = meta[0]>>7;
    int packetOP = meta[0]&15;

    byteArray thisChunk; // finalData = 1 piece of message
    thisChunk.assign(clientData.recvBuffer.begin(),clientData.recvBuffer.begin()+sz+metaLen);
    clientData.recvBuffer.erase(clientData.recvBuffer.begin(),clientData.recvBuffer.begin()+sz+metaLen);

    if(packetOP&8) {
        logger << "Control frame detected" << endl;
        if(packetOP == 8) { // opcode for closing connection , reading closing code
            int closingCode = ((cvt(thisChunk[nowPtr])^meta[(nowPtr-metaLen)%4+2])<<8) + (cvt(thisChunk[nowPtr+1])^meta[(nowPtr-metaLen+1)%4+2]);
            pData.data.push_back(wchar_t(closingCode));
            pData.opcode = "CLOS";
            logger << "Control OP 8 : Closing connection" << endl;
            return true;
        }
    }
    while(nowPtr < thisChunk.size()) { // demasked data and append to recvBuffer
        charcode = cvt(thisChunk[nowPtr])^meta[(nowPtr-metaLen)%4 + 2];
        clientData.waitingBuffer.push_back(charcode);
        nowPtr++;
    }
    if(packetFIN == 0) {
        logger << "FIN = 0 (Not finished packet)" << endl;
        if(packetOP == 0) {
            logger << "Continuation of old packet" << endl;
        }
        else {
            logger << "New packet with OP = " << packetOP << endl;
        }
        return false;
    }
    byteArray finalData; // finalData = 1 piece of message
    finalData.assign(clientData.waitingBuffer.begin(),clientData.waitingBuffer.end());
    clientData.waitingBuffer.clear();

    // use final data as demasked data , parsing with packet spec
    nowPtr = 0;
    int encodingType = finalData[nowPtr++];
    pData.enc = char(encodingType);
    int c;
    while((c = finalData[nowPtr]) != int('|')) {
        pData.opcode.push_back(char(c));
        nowPtr++;
        if(nowPtr >= finalData.size()) {
            logger << "Invalid PalmOS packet : OPCODE not finished" << endl;
            return false; // invalid packet spec
        }
    }
    nowPtr++; // skip |
    while((c = finalData[nowPtr]) != int('|')) {
        pData.id.push_back(char(c));
        nowPtr++;
        if(nowPtr >= finalData.size()) {
            logger << "Invalid PalmOS packet : ID not finished" << endl;
            return false; // invalid packet spec
        }
    }
    nowPtr++; // skip second |
    if(encodingType == int('1')) { // 1-byte encoding ( normal string )
        while(nowPtr < finalData.size()) {
            charcode = finalData[nowPtr];
            pData.data.push_back(wchar_t(charcode));
            pData.trimData.push_back(charcode);
            nowPtr++;
        }
    }
    else if(encodingType == int('2')) { // 2-bytes encoding ( wstring )
        while(nowPtr < finalData.size()) {
            if(nowPtr == finalData.size()-1) break; // shouldn't happen
            charcode = (finalData[nowPtr]<<8) + finalData[nowPtr+1];
            pData.data.push_back(wchar_t(charcode));
            pData.trimData.push_back(charcode>>8);
            pData.trimData.push_back(charcode%256);
            nowPtr += 2;
        }
    }
    logger << "Parsed successfully" << endl;
    return true;
}
byteArray wsEncodeMsg(string opcode,string data,string id="0",int type=WS_OP_TXT,char encodingType='2')
{
    byteArray toRet;
    unsigned long long int sz = data.size()+opcode.size()+1+1+id.size()+1; // ENC OPCODE | ID | DATA
    toRet.push_back(type);
    if(sz <= 125) { // normal len
        toRet.push_back(sz);
    }
    else if(sz <= 65535) { // 2 bytes len
        toRet.push_back(126);
        toRet.push_back((sz >> 8) &255);
        toRet.push_back(sz &255);
    }
    else { // 8 bytes len
        toRet.push_back(127);
        toRet.push_back((sz >> 56) &255);
        toRet.push_back((sz >> 48) &255);
        toRet.push_back((sz >> 40) &255);
        toRet.push_back((sz >> 32) &255);
        toRet.push_back((sz >> 24) &255);
        toRet.push_back((sz >> 16) &255);
        toRet.push_back((sz >> 8) &255);
        toRet.push_back(sz &255);
    }
    // DATA area
    toRet.push_back(cvt(encodingType));
    toRet.insert(toRet.end(),opcode.begin(),opcode.end());
    toRet.push_back('|');
    toRet.insert(toRet.end(),id.begin(),id.end());
    toRet.push_back('|');
    toRet.insert(toRet.end(),data.begin(),data.end());
    return toRet;
}

string reqClientInfo(int id) {
    stringstream ss;
    ss << "[[" << id << " " << clientList[id].ip;
    if(clientList[id].appname.size() > 0) ss << " " << translate_ws_to_s1(clientList[id].appname);
    ss << "]]";
    return ss.str();
}
/// MAIN LOOP FUNCTION
void run() {
    timestamp++;
    if(timestamp >= 1000000) timestamp = 0;
}
void loadDLL(string path)
{
    moduleS emp;
    emp.hin = LoadLibrary(path.c_str());
    moduleList.push_back(emp);
    moduleS* modPtr = &moduleList[moduleList.size()-1];
    logger << "Loading from : " << path << endl;
    if (modPtr->hin  == NULL) {
        logger << "Load failed" << endl;
        return;
    }
    logger << "Found module name : " << ((modf_who)GetProcAddress(modPtr->hin, "who"))() << endl;

    /// load module
    logger << "Initializing ... Requirement as follow : " << endl;
    vector<string> V = ((modf_load)GetProcAddress(modPtr->hin, "load"))();
    // run V
    logger.startLog("");
    for(int i = 0;i < V.size();i++) {
        logger << V[i] << endl;
        vector<string> loadArgs = split(V[i]);
        if(loadArgs[0].compare("OPNAME") == 0) {
            for(int j = 1;j < loadArgs.size();j++) {
                // register module's opname
                moduleMap.insert(pair<string,moduleS*>(loadArgs[j],modPtr));
                modPtr->moduleList.push_back(loadArgs[j]);
            }
        }
        else if(loadArgs[0].compare("FLAG") == 0) {
            for(int j = 1;j < loadArgs.size();j++) {
                // register module's flag
            }
        }
    }
    logger.endLog();
    modPtr->who = ((modf_who)GetProcAddress(modPtr->hin, "who"))();
    modPtr->version = ((modf_version)GetProcAddress(modPtr->hin, "version"))();

    modPtr->func_exec = (modf_exec)GetProcAddress(modPtr->hin, "exec");
    modPtr->func_poll = (modf_poll)GetProcAddress(modPtr->hin, "poll");
    modPtr->func_unload = (modf_unload)GetProcAddress(modPtr->hin, "unload");
    logger << "Module loaded successfully" << endl;
}
void unloadDLL(string name)
{

}
void recv(byteArray data,int i) {
    string str = toString(data);
    if(str.find("GET /") == 0 && str.find("HTTP/1.1") != string::npos) { /// WebServer
        if(str.find("Upgrade: websocket") != string::npos) { // WebSocket handshake
            server.sendTo(wsHandshake(str),i);
            logger.startLog("CONN");
            logger << "WebSocket connected with " << reqClientInfo(i) << endl;
            logger.endLog();
        }
        else { /// HTTP request
            string pageRequest = str.substr(5,str.find("HTTP/1.1")-6);
            if(pageRequest.size() == 0) pageRequest = "index.html";
            logger.startLog("HTTP");
            logger << "From : " << reqClientInfo(i) << endl;
            logger.startLog("");
            logger << "Request page : " << pageRequest << endl;

            string fullpath = workingDir;
            fullpath.append("\\");
            fullpath.append(pageRequest);
            string response;
            if(pageRequest.find_last_of('.') == string::npos) {
                logger << "Invalid resource indication" << endl;
                response = "HTTP/1.1 404 Not Found\r\n";
                response.append("\r\n");
            }
            else {
                string extension = pageRequest.substr(pageRequest.find_last_of('.'));
                string contentType = "";
                FILE* f = fopen(fullpath.c_str(),"rb");
                if(f == NULL) {
                    response = "HTTP/1.1 404 Not Found\r\n\r\n";
                    logger << "Not found (404)" << endl;
                }
                else {
                    response = "HTTP/1.1 200 OK\r\n";
                    response.append("Content-Type: ");
                    if(contentTypeMap.count(extension) == 0) response.append("application/octet-stream"); // byte array of unknown thing
                    else response.append(contentTypeMap[extension]);
                    response.append("\r\n\r\n");
                    char c;
                    c=fgetc(f);
                    while(!feof(f)) {
                        response.push_back(cvt(c));
                        c=fgetc(f);
                    }
                    if(ferror(f) != 0) {
                        logger << "File read error : " << ferror(f) << ". Treated as not found (404)" << endl;
                        response = "HTTP/1.1 404 Not Found\r\n";
                    }
                    else {
                        logger << "File read OK" << endl;
                    }
                    fclose(f);
                }
            }
            server.sendTo(response,i);
            server.disconnect(i);
            logger.endLog();
            logger.endLog();
        }
    }
    else { /// PalmOS
        packetS pData;
        // append data to recvBuffer
        clientList[i].recvBuffer.insert(clientList[i].recvBuffer.end(),data.begin(),data.end());
        while(clientList[i].recvBuffer.size() > 0) {
            logger.startLog("PARSE");
            bool parseResult = wsDecodeMsg(clientList[i],pData);
            logger.endLog();
            if(parseResult == false) continue; // do nothing on this packet

            string opcode = pData.opcode;
            wstring decodeMsg = pData.data;
            logger.startLog("PACKET");
            logger << reqClientInfo(i) << " OP[" << opcode.substr(0,15) << "] PID[" << pData.id << "] DATA[" << translate_ws_to_s1(decodeMsg.substr(0,20)) << "] SIZE[" << decodeMsg.size() << "]" << endl;
            logger.endLog();
            if(opcode == "CLOS") {
                int closingCode = int(decodeMsg[0]);
                logger.startLog("CONN");
                logger << "Close packet from " << reqClientInfo(i) << " [Code " << closingCode << "]" << endl;
                logger.endLog();
                server.disconnect(i);
            }
            else if(opcode == "DLL") {
                logger.startLog("DLL");
                vector<string> args = split(pData.trimData);
                if(args[0] == "LOAD") {
                    if(args.size() != 2) continue;
                    loadDLL(args[1]);
                }
                else if(args[0] == "UNLOAD") {
                    // UNLOAD [who]
                }
                logger.endLog();
            }
            else if(opcode == "SERVDATA") {
                vector<string> args = split(pData.trimData);
                if(args[0] == "DLLDATA") {
                    /// DLL data , module/flag associated
                    /// File|M_FILE|F_FLAGDRIVECHANGE
                }
                else if(args[0] == "REGISOPNAME") {
                    /// Registered opname
                    /// [OP1]|[WHO1]|[OP2}|[WHO2] ...
                    map<string,moduleS*>::iterator it = moduleMap.begin();
                    stringstream ss;
                    while(it != moduleMap.end()) {
                        ss << it->first << '|' << it->second->who << '|';
                        ++it;
                    }
                    string s = ss.str();
                    s.erase(s.end()-1);
                    server.sendTo(wsEncodeMsg("REGISOPNAME",s,pData.id,WS_OP_TXT,'1'),i);
                }
                else if(args[0] == "CLIENTDATA") {
                    /// Connected client data
                }
            }
            else {
                /// check in module
                if(moduleMap.find(opcode) != moduleMap.end()) {
                    packetS ret = moduleMap[opcode]->func_exec(pData);
                    if(ret.opcode != "") {
                        /// send this packet back
                        server.sendTo(wsEncodeMsg(ret.opcode,translate_ws_to_s2(ret.data),ret.id,ret.type,ret.enc),i);
                    }
                }
            }
        }
    }
}
bool acc(net_ext_sock connector)
{
    logger.startLog("CONN");
    logger << "Add " << net_getIpFromHandle(connector.sockHandle) << " as id " << clientList.size() << endl;
    clientS clientData;
    clientData.ip = net_getIpFromHandle(connector.sockHandle);
    clientData.socket_idRunner = 1;
    clientList.push_back(clientData);
    logger.endLog();
    return true; // accept the connection
}
void dis(int id)
{
    logger.startLog("CONN");
    logger << "Client disconnect : " << reqClientInfo(id);
    // remove flag associate to this , re-id everything
    map<string,vector<int> >::iterator it = clientFlag.begin();
    while(it != clientFlag.end()) {
        vector<int> *targ = &(it->second);
        for(int i = 0;i < targ->size();i++) {
            if(targ->at(i) > id) targ->at(i)--; // re-id
            else if(targ->at(i) == id) {
                targ->erase(targ->begin()+i); // remove
                i--;
            }
        }
        if(targ->size() == 0) {
            clientFlag.erase(it++);
        }
        else {
            it++;
        }
    }
    // close socket ( SOCKET module , should delete )
    map<int,net_client_clientClass>::iterator itt = clientList[id].socketList.begin();
    while(itt != clientList[id].socketList.end()) {
        cout << "Force shutdown " << endl;
        (itt->second).forceShutdown();
        itt++;
    }
    clientList.erase(clientList.begin() + id);
    logger.endLog();
}

void networkHelper_error(string str) {
    logger.startLog("ERROR");
    logger << str << endl;
    logger.endLog();
}
void networkHelper_debug(string str) {
    logger.startLog("SERVERDEBUG");
    logger << str << endl;
    logger.endLog();
}

void startServer()
{
    server.init();
    int port;
    cout << "Please enter server port : ";
    cin >> port;
    server.setup(port,run,recv,networkHelper_error,acc,dis);
    server.setDebugFunc(networkHelper_debug);
    server.start();
    server.runLoop();
}
int main(int argc,char** argv)
{
    net_init();
    SetErrorMode(SEM_NOOPENFILEERRORBOX); // for drives detection
    timestamp = 0;

    /// Logging
    logger.allowLog("MAIN");
    logger.allowLog("DLL");
    logger.allowLog("ERROR");
    logger.allowLog("CONN");


    if(argc >= 1) {
        workingDir = argv[0];
        workingDir = workingDir.substr(0,workingDir.find_last_of('\\'));
        logger.startLog("MAIN");
        logger << "Working directory : " << workingDir << endl;
        logger.endLog();
    }

    initContentTypeMap();
    startServer();
    net_close();
    return 0;
}
