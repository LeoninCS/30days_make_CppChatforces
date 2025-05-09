#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

class User {
public:
    User(int id = -1, string name = "", string password = "", string state = "") : id(id), name(name), password(password), state(state) {}
    ~User() = default;

    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setPassword(string password) { this->password = password; }
    void setState(string state) { this->state = state; }

    int getId() const { return id; }
    string getName() const { return name; }
    string getPassword() const { return password; }
    string getState() const { return state; }
protected: 
    int id;
    string name;
    string password;
    string state;

};


#endif