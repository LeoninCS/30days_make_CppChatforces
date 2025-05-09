#include "ChatService.hpp"
#include "ChatServer.hpp"
#include "Group.hpp"
#include "User.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include "json.hpp"
#include <ctime>
#include <thread>
#include "public.hpp"
using json = nlohmann::json;
using namespace std;

//当前账户
User _currentUser;
//群组
vector<Group> groups;
//好友
vector<User> friends;
//菜单
void mainMenu();
//用户
void showCurrentUser();

void showGroups();
void showFriends();
//接收线程
void readTaskHandler(int clientfd);

//获取当前时间
string getCurrentTime();

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        cout << "Usage: " << argv[0] << " <ip> <port>" << endl;
        return -1;
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //clientfd
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd < 0)
    {
        cout << "socket error" << endl;
        return -1;
    }
    //设置地址
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);

    //连接服务器
    if (connect(clientfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        cout << "connect error" << endl;
        close(clientfd);
        exit(0);
    }

    while(true) {
        cout << "===========================" << endl;
        cout << "1. Login" << endl;
        cout << "2. Register" << endl;
        cout << "3. Exit" << endl;
        cout << "===========================" << endl;
        cout << "Please choose an option: ";
        int choice;
        cin >> choice;
        cin.get();
        switch(choice) {
            case 1: {
                cout << "input id: ";
                int id;
                cin >> id;
                cin.get();
                cout << "input password: ";
                string password;
                getline(cin, password);
                // Create a JSON object for login
                json js;
                js["msgid"] = LOGIN_MSG; // Assuming 1 is the message ID for login
                js["id"] = id;
                js["password"] = password;
                string request = js.dump(); 
                // Send the JSON object to the server
                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if(len == -1) {
                    cerr << "send reg respons error" << endl;
                } else {
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if(len == -1) {
                        cerr << "recv reg respons error" << endl;
                    } else {
                        json jsresponse = json::parse(buffer);
                        if(jsresponse["errno"].get<int>() != 0) {
                            cerr << jsresponse["errmsg"] << endl;
                        } else {
                            _currentUser.setId(jsresponse["id"].get<int>());
                            _currentUser.setName(jsresponse["name"]);
                            //好友
                            if(jsresponse.contains("friends")) {
                                vector<string> friendVec = jsresponse["friends"];
                                for(auto & userstr : friendVec) {
                                    json friendUserjs = json::parse(userstr);
                                    User user;
                                    user.setId(friendUserjs["id"].get<int>());
                                    user.setName(friendUserjs["name"]);   
                                    user.setState(friendUserjs["state"]);
                                    friends.push_back(user);                            
                                }
                            }
                            //群组
                            if(jsresponse.contains("groups")) {
                                vector<string> groupVec = jsresponse["groups"];
                                for(auto & groupstr:groupVec) {
                                    json groupjs = json::parse(groupstr);
                                    Group group;
                                    group.setId(groupjs["id"].get<int>());
                                    group.setName(groupjs["groupname"]);
                                    group.setDesc(groupjs["groupdesc"]);

                                    vector<string> groupusersstr = groupjs["groupuser"];
                                    for(auto &groupuserstr: groupusersstr) {
                                        json groupuserjs = json::parse(groupuserstr);
                                        GroupUser groupuser;
                                        groupuser.setId(groupuserjs["id"].get<int>());
                                        groupuser.setName(groupuserjs["name"]);
                                        groupuser.setRole(groupuserjs["role"]);
                                        groupuser.setState(groupuserjs["state"]);
                                        group.getUsers().push_back(groupuser);
                                    }
                                    groups.push_back(group);
                                }
                            }
                            showCurrentUser();
                            //离线消息
                            if(jsresponse.contains("offlinemsg")) {
                                vector<string> offlinemsg = jsresponse["offlinemsg"];
                                for(auto &msgstr: offlinemsg) {
                                    json msg = json::parse(msgstr);
                                    cout << msg["time"] << "[" << msg["id"] << "]" << msg["name"] << "said: " << msg["msg"] << endl;
                                }
                            }

                            std::thread readTask(readTaskHandler,clientfd);
                            readTask.detach();

                            mainMenu();
                        }
                    }
                }
                break;
            }
                
            case 2: {
                char name[50];
                char password[50];
                cout << "Username:";
                cin.getline(name, 50);
                cout << "Password:";
                cin.getline(password,50);
                json js;
                js["msgid"] = REG_MSG;
                js["name"] = name;
                js["password"] = password;
                string request = js.dump();
                //cout << "发送请求: " << request << endl;
                int len = send(clientfd,request.c_str(),strlen(request.c_str()) + 1, 0);
                if(len == -1) {
                    cerr << "send reg response error!" << endl;
                } else {
                    char buffer[1024];
                    len = recv(clientfd,buffer, 1024, 0);
                    if(len == -1) {
                        cerr << "recv reg response error!";
                    } else { 
                        string raw(buffer,len);
                        json jsresponse = json::parse(raw);
                        if(jsresponse["errno"].get<int>() != 0) {
                            cerr << "Already exist, reg error!" << endl;
                        } else {
                            cout << name << "have regist success,userid is :" << jsresponse["id"] << "don't forget it" << endl;
                        }
                    }
                }
                break;
            }
            case 3: {
                close(clientfd);
                exit(0);
            }
                cout << "Exiting..." << endl;
                close(clientfd);
                return 0;
            default: {
                cerr << "invalid error" << endl;
                break;
            }
        }
    }
    return 0;
}

void mainMenu() {

}
//用户
void showCurrentUser() {

}

void showGroups() {

}

void showFriends() {

}
//接收线程
void readTaskHandler(int clientfd){

}

//获取当前时间
string getCurrentTime() {
    return " ";
}