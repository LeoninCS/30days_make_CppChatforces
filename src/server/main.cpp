#include "ChatServer.hpp"
#include "ChatService.hpp"
#include <iostream>
#include <signal.h>
using namespace std;

//处理服务器ctrl+c信号
void resetHandler(int sig)
{
    ChatService::instance()->reset();
    exit(0);
}


int main(int argc, char* argv[])
{

    if (argc < 3)
    {
        cout << "Usage: " << argv[0] << " <ip> <port>" << endl;
        return -1;
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");
    
    server.start();
    loop.loop();

    return 0;
}