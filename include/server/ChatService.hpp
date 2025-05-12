#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpServer.h>
#include <unordered_map>
#include <functional>
#include "UserModel.hpp"
#include "FriendModel.hpp"
#include "OfflineMessageModel.hpp"
#include "GroupModel.hpp"
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
    //登出
    void logout(const TcpConnectionPtr& conn, const json& js, Timestamp time);
    //注册
    void reg(const TcpConnectionPtr& conn, const json& js, Timestamp time);
    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
    //一对一聊天
    void oneChat(const TcpConnectionPtr& conn, const json& js, Timestamp time);
    
    //添加好友
    void addFriend(const TcpConnectionPtr& conn, const json& js, Timestamp time);
    //创建群组
    void createGroup(const TcpConnectionPtr& conn, const json& js, Timestamp time);
    //加入群组
    void addGroup(const TcpConnectionPtr& conn, const json& js, Timestamp time);
    //群组聊天
    void groupChat(const TcpConnectionPtr& conn, const json& js, Timestamp time);
    //退出群组
    void quitGroup(const TcpConnectionPtr& conn, const json& js, Timestamp time);
    //异常重置
    void reset();
private:
    ChatService();
    //存取id和消息处理函数
    unordered_map<int, MsgHandler> _msgHandlerMap;

    //存储在线用户的连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    //互斥锁
    mutex _connMutex;

    //操作用户表的对象
    UserModel _userModel;
    OfflineMessageModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
};

#endif