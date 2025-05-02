#include "public.hpp"
#include "ChatService.hpp"
#include <muduo/base/Logging.h>
#include <iostream>
#include <string>
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
            LOG_INFO << "send message";
            conn->send(response.dump());
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