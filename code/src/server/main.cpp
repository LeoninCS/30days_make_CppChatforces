#include "ChatServer.hpp"
#include <iostream>
using namespace std;

int main()
{
    EventLoop loop;
    InetAddress addr("127.0.0.1", 8888);
    ChatServer server(&loop, addr, "ChatServer");
    
    server.start();
    loop.loop();

    return 0;
}