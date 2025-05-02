#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

//User表操作类
class UserModel
{
public:
    bool insert(User &user);
    //根据用户号码查询信息
    User query(int id);
    //更新用户状态
    bool updateState(User &user);
private:

};
#endif