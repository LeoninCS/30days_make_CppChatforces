#include "UserModel.hpp"
#include "db.h"
#include <iostream>

bool UserModel::insert(User &user) {
    char sql[1024] = {0};
    string state = user.getState();
    if (state != "online" && state != "offline") {
        state = "offline";  // 如果 state 无效，设置为默认值 'offline'
    }
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPassword().c_str(), state.c_str());
    MySQL mysql; 
    if (mysql.connect()) {
        std::cout << "MySQL connect success!" << std::endl;
        if (mysql.update(sql)) {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    } else {
            std::cout << "MySQL connect failed!" << std::endl;        
    }
    return false;
}
//debug
/*
bool UserModel::insert(User &user) {
    char sql[1024] = {0};
    
    if (state != "online" && state != "offline") {
        state = "offline";  // 如果 state 无效，设置为默认值 'offline'
    }
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPassword().c_str(), state.c_str());
    std::cout << "执行 SQL: " << sql << std::endl;

    MySQL mysql;
    if (mysql.connect()) {
        std::cout << "MySQL connected" << std::endl;
        if (mysql.update(sql)) {
            user.setId(mysql_insert_id(mysql.getConnection()));
            std::cout << "用户注册成功，id = " << user.getId() << std::endl;
            return true;
        } else {
            std::cerr << "SQL 执行失败" << std::endl;
        }
    } else {
        std::cerr << "MySQL 连接失败" << std::endl;
    }
    return false;
}

*/
User UserModel::query(int id) {
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);
    MySQL mysql;
    if (mysql.connect()) {
        std::cout << "MySQL connect success!" << std::endl;
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    } else {
        std::cout << "MySQL connect failed!" << std::endl;        
    }
    return User();
}

bool UserModel::updateState(User &user) {
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());
    MySQL mysql;
    if (mysql.connect()) {
        std::cout << "MySQL connect success!" << std::endl;
        if (mysql.update(sql)) {
            return true;
        }
    } else {
        std::cout << "MySQL connect failed!" << std::endl;        
    }
    return false;
}

void UserModel::resetState() {
    char sql[1024] = "update user set state = 'offline' where state = 'online'";
    MySQL mysql;
    if (mysql.connect()) {
        std::cout << "MySQL connect success!" << std::endl;
        if (mysql.update(sql)) {
            std::cout << "重置用户状态成功" << std::endl;
        }
    } else {
        std::cout << "MySQL connect failed!" << std::endl;        
    }
}