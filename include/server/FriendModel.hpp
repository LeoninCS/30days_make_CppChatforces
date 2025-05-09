#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <vector>
#include "User.hpp"
using namespace std;
class FriendModel
{
public:
    //添加好友
    void addFriend(int userId, int friendId);

    //返回好友列表
    vector<User> query(int userId);

};


#endif