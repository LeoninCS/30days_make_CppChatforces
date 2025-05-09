#include "public.hpp"
#include "ChatService.hpp"
#include "FriendModel.hpp"
#include "Group.hpp"
#include "GroupModel.hpp"
#include <muduo/base/Logging.h>
#include <iostream>
#include <string>
#include <ctime>
#include <mutex>
using namespace std;
using namespace muduo;


ChatService*  ChatService::instance(){
    static ChatService service;
    return &service;
}

ChatService::ChatService() {
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({GROUP_QUIT_MSG, std::bind(&ChatService::quitGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
}


//登录
void ChatService::login(const TcpConnectionPtr& conn, const json& js, Timestamp time) {
    int id = js["id"];
    string pwd = js["password"];

    User user = _userModel.query(id);
    if(user.getId()== id && user.getPassword() == pwd) {
        if(user.getState() == "online") {
            //用户已登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 4;
            response["errmsg"] = "user already online";
            conn->send(response.dump());
        } else {
            //记录连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }
            
            LOG_INFO << "set online";
            user.setState("online");
            _userModel.updateState(user);
            //登录成功
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            std::time_t now = std::time(nullptr);
            char buf[100];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

            response["time"] = buf;

            //将离线消息发送给用户
            vector<string> vec = _offlineMsgModel.query(id);
            while (!vec.empty()) {
                response["offlinemsg"] = vec.back();
                vec.pop_back();
                conn->send(response.dump());
            }
            //清除离线消息
            _offlineMsgModel.remove(id);
            conn->send(response.dump());

            //查询好友列表
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty()) {
                vector<string> friendList;
                for (const auto& friendUser : userVec) {
                    json friendInfo;
                    friendInfo["id"] = friendUser.getId();
                    friendInfo["name"] = friendUser.getName();
                    friendInfo["state"] = friendUser.getState();
                    friendList.push_back(friendInfo.dump());
                }
                response["friends"] = friendList;
            } else {
                response["friends"] = json::array();
            }
            //群组信息
            vector<Group> groupVec = _groupModel.queryGroup(id);
            if(!groupVec.empty()) {
                vector<string> groupList;
                for(auto& group: groupVec) {
                    json groupInfo;
                    groupInfo["id"] = group.getId();
                    groupInfo["groupname"] = group.getName();
                    groupInfo["groupdesc"] = group.getDesc();
                    vector<GroupUser> groupUserVec = group.getUsers();
                    vector<string> groupUserList;
                    for(auto& groupUser : groupUserVec) {
                        json userjs;
                        userjs["id"] = groupUser.getId();
                        userjs["name"] = groupUser.getName();
                        userjs["role"] = groupUser.getRole();
                        userjs["state"] = groupUser.getState();
                        groupUserList.push_back(userjs.dump());
                    }    
                    groupInfo["groupuser"] = groupUserList;
                }
                response["groups"] = groupList;
            } else {
                response["groups"] = json::array();
            }
        }
    } else {
        
        if(user.getId() != id) {
            //用户不存在
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "user not exist";
            conn->send(response.dump());
        } else if(user.getPassword() != pwd) {
            //密码错误
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 3;
            response["errmsg"] = "password error";
            conn->send(response.dump());
        }
    }

    
    LOG_INFO << "do login";
}
//注册
void ChatService::reg(const TcpConnectionPtr& conn, const json& js, Timestamp time) {
    std::cout << "进入 ChatService::reg()" << std::endl;
    string name = js["name"];
    string password = js["password"];
    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = _userModel.insert(user);
    if(state) {
        //成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    } else {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
        //失败
    }
}
//debug
/*
void ChatService::reg(const TcpConnectionPtr& conn, const json& js, Timestamp time) {
    std::cout << "进入 ChatService::reg()" << std::endl;
    try {
        string name = js.at("name").get<string>();
        string password = js.at("password").get<string>();
        std::cout << "注册用户名: " << name << ", 密码: " << password << std::endl;

        User user;
        user.setName(name);
        user.setPassword(password);

        bool state = _userModel.insert(user);
        std::cout << "注册数据库插入结果: " << (state ? "成功" : "失败") << std::endl;

        json response;
        response["msgid"] = REG_MSG_ACK;

        if (state) {
            response["errno"] = 0;
            response["id"] = user.getId();
        } else {
            response["errno"] = 1;
        }

        std::string res = response.dump();
        std::cout << "即将发送的响应: " << res << std::endl;
        conn->send(res + "\n");

    } catch (std::exception& e) {
        std::cerr << "reg() 异常: " << e.what() << std::endl;
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 2;
        conn->send(response.dump() + "\n");
    }
}*/


MsgHandler ChatService::getHandler(int msgid) {
    //如果没有对应的处理器，返回一个空的处理器
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end()) {
        string errstr = "msgid:" + to_string(msgid) + " can not find handler!";
        return [=](auto a, auto b, auto c) {
            LOG_ERROR << errstr;
        };
    }
    return _msgHandlerMap[msgid];
}

void ChatService::clientCloseException(const TcpConnectionPtr& conn) {
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it) {
            if (it->second == conn) {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    if (user.getId() != -1) {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

//一对一聊天
void ChatService::oneChat(const TcpConnectionPtr& conn, const json& js, Timestamp time) {
    int toid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end()) {
            //在线转发
            it->second->send(js.dump());
            return;
        } else {
            //离线存储
            _offlineMsgModel.insert(toid, js);
        }
    }
}

//重置状态信息，全部下线
void ChatService::reset() {
    _userModel.resetState();
}

//添加好友
void ChatService::addFriend(const TcpConnectionPtr& conn, const json& js, Timestamp time) {
    int userId = js["id"].get<int>();
    int friendId = js["friendId"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        _friendModel.addFriend(userId, friendId);
    }
}
//创建群组
void ChatService::createGroup(const TcpConnectionPtr& conn, const json& js, Timestamp time) {
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group group(-1,name,desc);
    if(_groupModel.createGroup(group)) {
        //创建成功
        json response;
        response["msgid"] = CREATE_GROUP_MSG_ACK;
        response["errno"] = 0;
        response["groupid"] = group.getId();
        conn->send(response.dump());
        //加入群组
        _groupModel.addGroup(userid, group.getId(), "creator");
    } else {
        //创建失败
        json response;
        response["msgid"] = CREATE_GROUP_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}
//加入群组
void ChatService::addGroup(const TcpConnectionPtr& conn, const json& js, Timestamp time) {
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    string role = "normal";
    if(_groupModel.addGroup(userid, groupid, role)) {
        json response;
    response["msgid"] = ADD_GROUP_MSG_ACK;
    response["errno"] = 0;
    conn->send(response.dump());
    } else {
        json response;
    response["msgid"] = ADD_GROUP_MSG_ACK;
    response["errno"] = 0;
    conn->send(response.dump());
    }
    
}
//群组聊天
void ChatService::groupChat(const TcpConnectionPtr& conn, const json& js, Timestamp time) {
    int groupid = js["groupid"].get<int>();
    int userid = js["id"].get<int>();
    string msg = js["msg"];
    vector<int> userVec = _groupModel.queryGroupUsers(userid, groupid);
    while(!userVec.empty()) {
        int id = userVec.back();
        userVec.pop_back();
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end()) {
            //在线转发
            it->second->send(js.dump());
        } else {
            //离线存储
            _offlineMsgModel.insert(id, js);
        }
    }
}
//退出群组
void ChatService::quitGroup(const TcpConnectionPtr& conn, const json& js, Timestamp time) {
    
}