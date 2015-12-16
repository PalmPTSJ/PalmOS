#include "network_server.h"
net_server_serverClass::net_server_serverClass() /** Constructor */
{
    sock = INVALID_SOCKET;
}
void net_server_serverClass::init() /** init default data */
{
    sock = net_createSocket();
    port = NET_DEFAULT_PORT;
    isStart = false;
    isShuttingDown = false;
    if(debugFunc != NULL) (*debugFunc)("Server init");
}
net_server_serverClass::~net_server_serverClass() /** Destructor */
{
    if(isStart) { // Force shutdown
        forceStop();
    }
}
void net_server_serverClass::setup(int _port,void(*run)(),void(*recv)(byteArray,int),void(*err)(string),bool(*acc)(net_ext_sock),void(*dis)(int))
{ /** Set up data (auto flow) before start */
    if(sock == INVALID_SOCKET) init();
    port = _port;
    runFunc = run;
    recvFunc = recv;
    errFunc = err;
    accFunc = acc;
    disFunc = dis;
}
void net_server_serverClass::setup(int _port)
{ /** Set up data (manual flow) */
    setup(_port,NULL,NULL,NULL,NULL,NULL);
}

bool net_server_serverClass::start()
{
    net_bindAndListen(sock,net_createAddr("",port)); // Bind to port , listen to any ip
    if(net_error()) return false;
    isStart = true;
    return true;
}

void net_server_serverClass::disconnect(int index)
{
    if(index < 0 || index >= clientList.size()) return;
    if(clientList[index].sendBuff.size() > 0)  // send any pending data
        for(int j = 0;j < clientList[index].sendBuff.size();j++)
            net_send(clientList[index].sockHandle.sock,clientList[index].sendBuff[j]);
    clientList[index].status = NET_EXT_SOCK_CLOSING;
    net_shutdownHandle(clientList[index].sockHandle);
}
void net_server_serverClass::stop()
{
    if(isShuttingDown) return;
    if(debugFunc != NULL) (*debugFunc)("Shutting down");
    for(int i = 0;i < clientList.size();i++) { /** Initialize graceful shutdown ( will wait until the other side close ) */
        clientList[i].status = NET_EXT_SOCK_CLOSING;
        net_shutdownHandle(clientList[i].sockHandle);
    }
    isShuttingDown = true;
}
void net_server_serverClass::forceStop()
{
    if(debugFunc != NULL) (*debugFunc)("Force close");
    for(int i = 0;i < clientList.size();i++) /** force close everything */
        net_closeHandle(clientList[i].sockHandle);
    net_closeSocket(sock);
    isStart = false;
}
int net_server_serverClass::run()
{
    for(int i = 0;i < clientList.size();i++) {
        if(clientList[i].sendBuff.size() > 0) { /// send waiting data
            for(int j = 0;j < clientList[i].sendBuff.size();j++) {
                net_send(clientList[i].sockHandle.sock,clientList[i].sendBuff[j]);
            }
            clientList[i].sendBuff.clear();
        }
        byteArray recvData;
        int ret = net_recv(clientList[i].sockHandle.sock,recvData);
        if(ret == NET_RECV_CLOSE && clientList[i].status == NET_EXT_SOCK_CLOSING) { /// shutdown handling
            char cc[100]; sprintf(cc,"%s [%d] disconnected (SHUTD)",net_getIpFromHandle(clientList[i].sockHandle).c_str(),i);
            if(debugFunc != NULL) (*debugFunc)(string(cc));
            net_closeHandle(clientList[i].sockHandle);
            if(disFunc != NULL) (*disFunc)(i);
            clientList.erase(clientList.begin() + i);
            --i;
        }
        else if(ret == NET_RECV_OK) { /// recieve data
            clientList[i].recvBuff.push_back(recvData);
        }
        else if(ret == NET_RECV_CLOSE) { /// other side disconnect
            char cc[100]; sprintf(cc,"%s [%d] disconnected (CSIGN)",net_getIpFromHandle(clientList[i].sockHandle).c_str(),i);
            if(debugFunc != NULL) (*debugFunc)(string(cc));
            net_closeHandle(clientList[i].sockHandle);
            if(disFunc != NULL) (*disFunc)(i);
            clientList.erase(clientList.begin() + i);
            --i;
        }
        else if(ret == NET_RECV_ERROR) {
            char cc[100]; sprintf(cc,"%s [%d] disconnected (ERROR)",net_getIpFromHandle(clientList[i].sockHandle).c_str(),i);
            if(debugFunc != NULL) (*debugFunc)(string(cc));
            net_closeHandle(clientList[i].sockHandle);
            if(disFunc != NULL) (*disFunc)(i);
            clientList.erase(clientList.begin() + i);
            --i;
        }
    }
    if(isShuttingDown && clientList.size() == 0) { /// graceful shutdown success
        net_closeSocket(sock);
        isStart = false;
        isShuttingDown = false;
        return NET_SERVER_STOPSUCCESS;
    }
    return 0;
}

int net_server_serverClass::acceptNewRequest()
{
    if(!isStart) return -1;
    if(isShuttingDown) return -1; // don't accept new request when shutting down
    net_sockHandle hnd = net_accept(sock);
    if(hnd.sock != INVALID_SOCKET) { /** Connection recieved */
        net_ext_sock client = net_ext_createSock();
        client.sockHandle = hnd;
        client.status = NET_EXT_SOCK_ONLINE;
        char cc[100]; sprintf(cc,"%s connected",net_getIpFromHandle(hnd).c_str());
        if(debugFunc != NULL) (*debugFunc)(string(cc));
        clientList.push_back(client);
        if(accFunc != NULL) if((*accFunc)(client) == false) {
            // reject this acception
            disconnect(clientList.size()-1);
            return 2; // connection rejected
        }
        return 1; // new connection recieved
    }
    return 0; // nothing new
}

bool net_server_serverClass::isClientHaveData(int index)
{
    return (clientList[index].recvBuff.size()>0)?true:false;
}

byteArray net_server_serverClass::getRecvDataFrom(int index)
{
    byteArray toRet;
    if(index < 0 || index >= clientList.size()) return toRet;
    if(clientList[index].recvBuff.size() == 0) return toRet; // no data
    toRet = clientList[index].recvBuff[0];
    clientList[index].recvBuff.erase(clientList[index].recvBuff.begin());
    return toRet;
}

string net_server_serverClass::getIpFrom(int index)
{
    if(index < 0 || index >= clientList.size()) return "";
    return net_getIpFromHandle(clientList[index].sockHandle);
}
void net_server_serverClass::sendTo(byteArray data,int index)
{
    if(index < 0 || index >= clientList.size()) return;
    clientList[index].sendBuff.push_back(data);
}
void net_server_serverClass::sendTo(string data,int index)
{
    sendTo(toByteArray(data),index);
}
void net_server_serverClass::sendToAllClient(byteArray data)
{
    for(int i = 0;i < clientList.size();i++)
        clientList[i].sendBuff.push_back(data);
}
void net_server_serverClass::sendToAllClient(string data)
{
    sendToAllClient(toByteArray(data));
}
void net_server_serverClass::sendToAllClientExcept(byteArray data,int exceptIndex)
{
    for(int i = 0;i < clientList.size();i++)
        if(i != exceptIndex)
            clientList[i].sendBuff.push_back(data);
}
void net_server_serverClass::sendToAllClientExcept(string data,int exceptIndex)
{
    sendToAllClientExcept(toByteArray(data),exceptIndex);
}

void net_server_serverClass::setDebugFunc(void(*f)(string))
{
    debugFunc = f;
}

void net_server_serverClass::runLoop()
{
    if(!isStart) {
        if(!start()) { // start error
            if(errFunc != NULL) (*errFunc)(net_lastError);
            return;
        }
    }
    if(debugFunc != NULL) (*debugFunc)("Run Loop started");
    while(isStart) {
        --delay;
        if(delay <= 0) {
            delay = NET_SERVER_ACCEPT;
            acceptNewRequest();
        }
        if(delay % NET_SERVER_RECV == 0) {
            int runStat = run(); // run for recieving data
            if(isShuttingDown && (runStat == NET_SERVER_STOPSUCCESS)) break; // shutdown completed
            for(int i = 0;i < clientList.size();i++) {
                while(isClientHaveData(i)) { /* if this client have data , call recv function */
                    byteArray recvData = getRecvDataFrom(i);
                    if(recvFunc != NULL) (*recvFunc)(recvData,i);
                }
            }
        }
        if(net_error()) /* If have error */
        {
            if(errFunc != NULL) (*errFunc)(net_lastError);
            break; /* Force exit */
        }
        if(delay % NET_SERVER_RUN == 0) {
            if(runFunc != NULL) (*runFunc)();
        }
        Sleep(NET_SERVER_SLEEP);
    }
}
