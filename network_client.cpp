#include "network_client.h"

net_client_clientClass::net_client_clientClass()
{
    status = NET_CLIENT_OFFLINE;
    sock = INVALID_SOCKET;
    runFunc = NULL;
    recvFunc = NULL;
    errFunc = NULL;
    debugFunc = NULL;
}
net_client_clientClass::~net_client_clientClass()
{
    if(sock != INVALID_SOCKET) net_closeSocket(sock);
}
void net_client_clientClass::setup(void(*run)(),void(*recv)(byteArray),void(*err)(string))
{
    if(debugFunc != NULL) (*debugFunc)("Setup");
    runFunc = run;
    recvFunc = recv;
    errFunc = err;
}
bool net_client_clientClass::connect(string ip,int port,int timeout)
{
    if(sock == INVALID_SOCKET) {
        sock = net_createSocket();
    }
    if(debugFunc != NULL) (*debugFunc)("Connecting");
    bool con = net_connect(sock,net_createAddr(ip,port),timeout);
    if(!con) {
        if(net_error()) { if(errFunc != NULL) (*errFunc)(net_lastError); }
        else if(errFunc != NULL) (*errFunc)("Connection Error");
        return false;
    }
    else {
        status = NET_CLIENT_ONLINE;
        return true;
    }
}
int net_client_clientClass::recvData(byteArray& data)
{
    if(status != NET_CLIENT_ONLINE && status != NET_CLIENT_SHUTTINGDOWN) {
        //cout << "Socket not online" << endl;
        return -1;
    }
    if(status == NET_CLIENT_SHUTTINGDOWN) {
        cout << "Socket is shutting down " << endl;
    }
    if(sock == INVALID_SOCKET) {
        cout << "INV socket" << endl;
    }
    //cout << "I am recieving" << endl;
    int stat = net_recv(sock,data);
    if(stat == NET_RECV_CLOSE || stat == NET_RECV_ERROR)
    {
        //cout << "I recieved close" << endl;
        // close
        if(debugFunc != NULL) (*debugFunc)("Disconnected (CSIGN)");
        if(status == NET_CLIENT_SHUTTINGDOWN) {
            (*debugFunc)("Graceful shutdown completed (CSIGN)");
            /*net_closeSocket(sock);
            sock = INVALID_SOCKET;*/
        }
        net_closeSocket(sock);
        sock = INVALID_SOCKET;
        status = NET_CLIENT_OFFLINE;
        return 2;
    }
    else if(stat == NET_RECV_OK) {
        //cout << "I recieved data" << endl;
        return 1;
    }
    if(net_error()) /* If have error */
    {
        if(errFunc != NULL) (*errFunc)(net_lastError);
        status = NET_CLIENT_OFFLINE;
        return -1; /* Force exit */
    }
    //cout << "I recieved nothing" << endl;
    return 0;
}
bool net_client_clientClass::run()
{
    if(status != NET_CLIENT_ONLINE && status != NET_CLIENT_SHUTTINGDOWN) return false;
    --delay;
    if(delay <= 0) {
        delay = NET_CLIENT_DELAY;
        byteArray data;
        int stat = net_recv(sock,data);
        if(stat == NET_RECV_CLOSE || stat == NET_RECV_ERROR)
        {
            // close
            if(debugFunc != NULL) (*debugFunc)("Disconnected (CSIGN)");
            if(status == NET_CLIENT_SHUTTINGDOWN) {
                (*debugFunc)("Graceful shutdown completed (CSIGN)");
                net_closeSocket(sock);
                sock = INVALID_SOCKET;
            }
            status = NET_CLIENT_OFFLINE;
            return false;
        }
        else if(stat == NET_RECV_OK) {
            if(recvFunc != NULL) (*recvFunc)(data);
        }
    }
    if(net_error()) /* If have error */
    {
        if(errFunc != NULL) (*errFunc)(net_lastError);
        status = NET_CLIENT_OFFLINE;
        return false; /* Force exit */
    }
    if(runFunc != NULL) (*runFunc)();
    return true;
}
void net_client_clientClass::runLoop()
{
    if(status == NET_CLIENT_OFFLINE) {
        return;
    }
    if(debugFunc != NULL) (*debugFunc)("Run Loop started");
    while(status == NET_CLIENT_ONLINE) {
        if(run() == false) break;
        Sleep(NET_CLIENT_SLEEP);
    }
}
void net_client_clientClass::send(byteArray data)
{
    net_send(sock,data);
}
void net_client_clientClass::send(string data)
{
    send(toByteArray(data));
}
void net_client_clientClass::disconnect()
{
    shutdown(sock,SD_SEND);
    status = NET_CLIENT_SHUTTINGDOWN;
}
void net_client_clientClass::forceShutdown()
{
    shutdown(sock,SD_SEND);
    status = NET_CLIENT_OFFLINE;
    closesocket(sock);
}
void net_client_clientClass::setDebugFunc(void(*f)(string))
{
    debugFunc = f;
}
