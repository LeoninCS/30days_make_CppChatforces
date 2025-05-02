#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    //初始化聊天服务器
    ChatServer (EventLoop *loop,
         const InetAddress& listenAddr,
         const string& nameArg);
    //启动服务     
    void start();
private:
    //回调连接信息
    void onConnection(const TcpConnectionPtr& conn);
    //回调消息
    void onMessage(const TcpConnectionPtr& conn,
                   Buffer* buffer,
                   Timestamp time);
    TcpServer _server;
    EventLoop *_loop;
    
};


#endif
