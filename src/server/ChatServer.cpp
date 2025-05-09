#include "ChatServer.hpp"
#include "ChatService.hpp"
#include "json.hpp"
#include <functional>
#include <string>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

//初始化聊天服务器
ChatServer::ChatServer(EventLoop *loop,
    const InetAddress& listenAddr,
    const string& nameArg) : _server(loop, listenAddr, nameArg), _loop(loop) {
        //注册链接回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
        
        //注册消息回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        
        //设置线程数
        _server.setThreadNum(4);
    }
//启动服务
void ChatServer::start() {
    //启动服务
    _server.start();
}

//回调连接信息
void ChatServer::onConnection(const TcpConnectionPtr& conn) {
    //客户端断开连接
    if(!conn->connected()) {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

//回调消息
void ChatServer::onMessage(const TcpConnectionPtr& conn,
    Buffer* buffer,
    Timestamp time) {
        string buf = buffer->retrieveAllAsString();
        //解析json
        json js = json::parse(buf);
        //获取消息类型
        auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
        msgHandler(conn, js, time);
    }