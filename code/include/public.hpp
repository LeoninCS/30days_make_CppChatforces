#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType
{
    LOGIN_MSG = 1, //登录消息
    LOGIN_MSG_ACK, //登录消息响应
    REG_MSG, //注册消息
    REG_MSG_ACK, //注册消息响应
    ONE_CHAT_MSG, //一对一聊天消息
    ADD_FRIEND_MSG //添加好友消息
};

#endif