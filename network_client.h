#ifndef NETWORK_CLIENT_H_INCLUDED
#define NETWORK_CLIENT_H_INCLUDED

#include "network_ext.h"

#define NET_CLIENT_ONLINE 1
#define NET_CLIENT_OFFLINE 0
#define NET_CLIENT_SHUTTINGDOWN 2

#define NET_CLIENT_SLEEP 16 // time sleep
#define NET_CLIENT_DELAY 1

class net_client_clientClass
{
public:
    net_client_clientClass();
    ~net_client_clientClass();

    void setup(void(*run)(),void(*recv)(byteArray),void(*err)(string)); // Using automatic flow
    void setDebugFunc(void(*f)(string));

    bool connect(string ip,int port,int timeout);

    bool run();
    void runLoop();

    int recvData(byteArray& data);

    void send(byteArray data);
    void send(string data);

    void disconnect();
    void forceShutdown();

    void (*debugFunc)(string);
    void (*runFunc)();
    void (*recvFunc)(byteArray);
    void (*errFunc)(string);

    int delay;
    int status;

    SOCKET sock;
};

#endif // NETWORK_CLIENT_H_INCLUDED
