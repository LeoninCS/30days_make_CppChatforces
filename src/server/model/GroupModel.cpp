#include "GroupModel.hpp"
#include "Group.hpp"
#include "db.h"

 //创建群组
 bool GroupModel::createGroup(Group &group) {
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname,groupdesc) values('%s','%s')", group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if(mysql.connect()) {
        if(mysql.update(sql)) {
            return true;
        }
    }
    return false;
 }
 //加入群组
 bool GroupModel::addGroup(int userid, int groupid, string role) {
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser(groupid,userid,role) values('%d','%d','%s')",groupid,userid,role.c_str());
    MySQL mysql;
    if(mysql.connect()) {
        if(mysql.update(sql)) {
            return true;
        }
    }
    return false;
 }


 //查询群组信息
 vector<Group> GroupModel::queryGroup(int userId) {
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup a inner join \
        groupuser b on a.id = b.groupid where b.userid = %d", userId);
        vector<Group> groupVec;
        MySQL mysql;
        if(mysql.connect()) {
            MYSQL_RES * res = mysql.query(sql);
            if(res != nullptr) {
                MYSQL_ROW row;
                while((row = mysql_fetch_row(res)) != nullptr) {
                    Group group;
                    group.setId(atoi(row[0]));
                    group.setName(row[1]);
                    group.setDesc(row[2]);
                    groupVec.push_back(group);
                }
                mysql_free_result(res);
            }
        }
        for(Group &group : groupVec) {
            sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a inner join \
                groupuser b on b.userid = a.id where b.groupid = %d", group.getId());
            if(mysql.connect()) {
                MYSQL_RES * res = mysql.query(sql);
                if(res != nullptr) {
                    MYSQL_ROW row;
                    while((row = mysql_fetch_row(res)) != nullptr) {
                        GroupUser user;
                        user.setId(atoi(row[0]));
                        user.setName(row[1]);
                        user.setState(row[2]);
                        user.setRole(row[3]);
                        group.getUsers().push_back(user);
                    }
                    mysql_free_result(res);
                }
            }
        }
        return groupVec;
    }
 //群发消息
 vector<int> GroupModel::queryGroupUsers(int userId,int groupId) {
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != '%d'", groupId,userId);
    MySQL mysql;
    vector<int> userVec;
    if(mysql.connect()) {
        MYSQL_RES * res = mysql.query(sql);
        if(res != nullptr) {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr) {
                userVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return userVec;
 }