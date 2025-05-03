#ifndef OFFLINEMESSAGE_H
#define OFFLINEMESSAGE_H
#include <string>
#include <vector>
using namespace std;
class OfflineMessageModel {
public:
    //存储离线消息
    void insert(int id, const string& msg);
    //删除离线消息
    void remove(int id);
    //查询离线消息
    vector<string> query(int id);
};

#endif