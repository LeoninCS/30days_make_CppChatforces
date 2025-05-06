#ifndef GROUP_HPP
#define GROUP_HPP
#include "GroupUser.hpp"
#include <string>
#include <vector>
using namespace std;

class Group {

public:
    Group(int id = -1, const string& name = "", const string& desc = ""){
        this->id = id;
        this->name = name;
        this->desc = desc;
    }
    int getId() {return id;}
    string getName() {return name;}
    string getDesc() {return desc;}
    void setId(int id) {this->id = id;}
    void setName(const string& name) {this->name = name;}
    void setDesc(const string& desc) {this->desc = desc;}
    vector<GroupUser>& getUsers() {return users;}

private:
    int id;
    string name;
    string desc;
    vector<GroupUser> users;
};


#endif