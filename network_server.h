#ifndef NETWORK_SERVER_H_INCLUDED
#define NETWORK_SERVER_H_INCLUDED

#include "network_ext.h"

#define NET_SERVER_STOPSUCCESS 1
#define NET_SERVER_SLEEP 16 // time sleep
#define NET_SERVER_ACCEPT 30 // delay count until accept
#define NET_SERVER_RUN 2 // delay count until run , must < accept
#define NET_SERVER_RECV 1 // delay count until recv , must < accept
class net_server_serverClass
{
public:
    net_server_serverClass();
    ~net_server_serverClass();

    void init();

    void setup(int _port); // Using manual flow
    void setup(int _port,void(*run)(),void(*recv)(byteArray,int),void(*err)(string),bool(*acc)(net_ext_sock),void(*dis)(int)); // Using automatic flow

    bool start();

    int acceptNewRequest(); // Check for new request

    int run(); // Manual flow recieving
    void runLoop(); // Automatic flow recieving & accept & error

    void disconnect(int index);

    void stop(); // graceful shutdown
    void forceStop();

    byteArray getRecvDataFrom(int index);
    bool isClientHaveData(int index);
    string getIpFrom(int index);

    void sendTo(byteArray data,int index);
    void sendTo(string data,int index);
    void sendToAllClient(byteArray data);
    void sendToAllClient(string data);
    void sendToAllClientExcept(byteArray data,int exceptIndex);
    void sendToAllClientExcept(string data,int exceptIndex);

    void setDebugFunc(void(*f)(string));

    string getError();

    SOCKET sock;
    bool isStart;
    bool isShuttingDown;
    int port;
    string error;
    vector<net_ext_sock> clientList;
    void (*debugFunc)(string);
    void (*runFunc)();
    void (*recvFunc)(byteArray,int);
    void (*errFunc)(string);
    bool (*accFunc)(net_ext_sock);
    void (*disFunc)(int);
    int delay;
};

#endif // NETWORK_SERVER_H_INCLUDED
