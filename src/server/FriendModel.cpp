#include "FriendModel.hpp"
#include "db.h"
#include <iostream>
using namespace std;

//添加好友
void FriendModel::addFriend(int userId, int friendId) {
    char sql[1024] = {0};
    sprintf(sql, "insert into friend(userId, friendId) values(%d, %d)", userId, friendId);
    MySQL mysql;
    if (mysql.connect()) {
        std::cout << "MySQL connect success!" << std::endl;
        if (mysql.update(sql)) {
            std::cout << "添加好友成功" << std::endl;
        } else {
            std::cerr << "添加好友失败" << std::endl;
        }
    } else {
        std::cerr << "MySQL connect failed!" << std::endl;
    }
}

//返回好友列表
vector<User> FriendModel::query(int userId) {
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userId = %d", userId);
    MySQL mysql;
    vector<User> vec;
    if (mysql.connect()) {
        std::cout << "MySQL connect success!" << std::endl;
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        } else {
            std::cerr << "查询好友列表失败" << std::endl;
        }
    } else {
        std::cerr << "MySQL connect failed!" << std::endl;
    }
    return vec;
}