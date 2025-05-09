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


int main()
{
    EventLoop loop;
    InetAddress addr("127.0.0.1", 8888);
    ChatServer server(&loop, addr, "ChatServer");
    
    server.start();
    loop.loop();

    return 0;
}