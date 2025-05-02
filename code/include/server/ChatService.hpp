#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpServer.h>
#include <unordered_map>
#include <functional>
#include "UserModel.hpp"
#include "json.hpp"
#include <string>
#include <mutex>

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

using MsgHandler = std::function<void(const TcpConnectionPtr& conn, const json& js, Timestamp)>;

class ChatService
{
public:
    //获取单例对象
    static ChatService* instance();
    //登录
    void login(const TcpConnectionPtr& conn, const json& js, Timestamp time);
    //注册
    void reg(const TcpConnectionPtr& conn, const json& js, Timestamp time);
    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
private:
    ChatService();
    //存取id和消息处理函数
    unordered_map<int, MsgHandler> _msgHandlerMap;

    //存储在线用户的连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    //互斥锁
    mutex _connMutex;

    UserModel _userModel;
};

#endif