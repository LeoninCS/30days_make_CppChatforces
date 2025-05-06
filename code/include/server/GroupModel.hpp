#ifndef GROUPMODEL_HPP
#define GROUPMODEL_HPP
#include "Group.hpp"
#include <string>
#include <vector>
using namespace std;
class GroupModel {
public:
    //创建群组
    bool createGroup(Group &group);
    //加入群组
    bool addGroup(int userid, int groupid, string role);
    //查询群组信息
    vector<Group> queryGroup(int userId);
    //获取群组成员
    vector<int> queryGroupUsers(int userId,int groupId);
};
#endif