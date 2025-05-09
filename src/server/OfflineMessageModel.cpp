#include "OfflineMessageModel.hpp"
#include "db.h"

//存储离线消息
void OfflineMessageModel::insert(int id, const string& msg) {
    char sql[1024] = {0};
    sprintf(sql, "insert into offline_message values(%d, '%s')", id, msg.c_str());
    MySQL mysql;
    if(mysql.connect()) {
        mysql.update(sql);
        return;
    }
}
//删除离线消息
void OfflineMessageModel::remove(int id) {
    char sql[1024] = {0};
    sprintf(sql, "delete from offline_message where userid = %d", id);
    MySQL mysql;
    if(mysql.connect()) {
        mysql.update(sql);
        return;
    }
}
//查询离线消息
vector<std::string> OfflineMessageModel::query(int id) {
    char sql[1024] = {0};
    sprintf(sql, "select message from offline_message where userid = %d", id);
    MySQL mysql;
    vector<string> vec;
    if(mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if(res) {
            MYSQL_ROW row;
            
            while((row = mysql_fetch_row(res)) != nullptr) {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return vec;;
}