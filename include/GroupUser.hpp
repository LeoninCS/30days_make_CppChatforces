#ifndef GROUPUSER_HPP
#define GROUPUSER_HPP
#include "User.hpp"
#include <string>
using namespace std;
class GroupUser : public User {
public:
    void setRole(string role) { this->role = role; }
    string getRole() const { return role; } 
private:
    string role;
};

#endif