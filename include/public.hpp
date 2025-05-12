#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType
{
    LOGIN_MSG = 1, //登录消息
    LOGIN_MSG_ACK, //登录消息响应
    LOGOUT_MSG, //登出消息
    REG_MSG, //注册消息
    REG_MSG_ACK, //注册消息响应
    ONE_CHAT_MSG, //一对一聊天消息
    ADD_FRIEND_MSG, //添加好友消息
    ADD_FRIEND_MSG_ACK, //添加好友消息响应
    CREATE_GROUP_MSG, //创建群组消息
    CREATE_GROUP_MSG_ACK, //创建群组消息响应
    ADD_GROUP_MSG, //加入群组消息
    ADD_GROUP_MSG_ACK, //加入群组消息响应
    GROUP_CHAT_MSG, //群组聊天消息
    GROUP_QUIT_MSG, //退出群组消息
};

#endif