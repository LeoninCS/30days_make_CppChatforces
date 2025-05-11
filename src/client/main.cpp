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
#include <sstream>
#include "public.hpp"
#include <unordered_map>
using json = nlohmann::json;
using namespace std;

//当前账户
User _currentUser;
//群组
vector<Group> groups;
//好友
vector<User> friends;
//菜单
void mainMenu(int);
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
                    cerr << "send login respons error" << endl;
                } else {
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if(len == -1) {
                        cerr << "recv login respons error" << endl;
                    } else {
                        json jsresponse = json::parse(buffer);
                        if(jsresponse["errno"].get<int>() != 0) {
                            cerr << jsresponse["errmsg"] << endl;
                        } else {
                            _currentUser.setId(jsresponse["id"].get<int>());
                            _currentUser.setName(jsresponse["name"]);
                            cout << getCurrentTime() << endl;
                            cout << "id: " <<_currentUser.getId() << endl;
                            cout << "name: " << _currentUser.getName() << endl;
                            cout << "this user longins success!" << endl;  
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

                            mainMenu(clientfd);
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

void help(int = 0, string = "");
void chat(int, string);
void addfriend(int, string);
void creategroup(int,string);
void addgroup(int, string);
void groupchat(int, string);
void loginout(int = 0, string = "");

unordered_map<string,string> commandMap = {
    {"help","显示所有支持的命令,格式help"},
    {"chat","一对一聊天,格式chat:friendid:message"},
    {"addfriend","添加好友,格式addfriend:friendid"},
    {"creategroup","创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup","加入群组,格式addgroup:groupid"},
    {"groupchat","群聊,格式groupchat:groupid:message"},
    {"loginout","注销登录,格式loginout"}
};

unordered_map<string,function<void(int, string)>> commandHandlerMap = {
    {"help",help},
    {"chat",chat},
    {"addfriend",addfriend},
    {"creategroup",creategroup},
    {"addgroup",addgroup},
    {"groupchat",groupchat},
    {"loginout",loginout}
};

void mainMenu(int clientfd) {
    help();
    char buffer[1024] = {0};
    while(true) {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(":");
        if(idx == -1) {
            command = commandbuf;
        } else {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end()) {
            cerr << "invalid input command!" << endl;
            continue;
        }
        it->second(clientfd,commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }
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
    while(true) {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if(len == -1 || len == 0) {
            close(clientfd);
            exit(-1);
        }
        json js = json::parse(buffer);
        if(js["msgid"].get<int>() == ONE_CHAT_MSG) {
            cout << js["time"].get<string>() << " [" << js["id"] << " ]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
    }
}

//获取当前时间
string getCurrentTime() {
    auto now = std::chrono::system_clock::now();               
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::tm local_tm;
    localtime_r(&now_c, &local_tm);

    std::ostringstream oss;
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local_tm);
    oss << std::string(buffer); // 格式化输出
    return oss.str();
}

void help(int, string) {
    for(auto &[a,b]:commandMap) {
        cout << a << "----" << b << endl;
    }
    cout << endl;
}

void addfriend(int clientfd, string str) {
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = _currentUser.getId();
    js["name"] = _currentUser.getName();
    js["friendId"] = clientfd;
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if(len == - 1) {
        cerr << "send addfriend msg error -> " << buffer << endl; 
    }
}

void chat(int clientfd, string str) {
    int idx = str.find(":");
    if(idx == -1) {
        cerr << "message is not true" << endl;
    } else {
        int friendId = atoi(str.substr(0, idx).c_str());
        string message = str.substr(idx + 1,(str.size() - idx));
        json js;
        js["msgid"] = ONE_CHAT_MSG;
        js["id"] = _currentUser.getId();
        js["name"] = _currentUser.getName();
        js["toid"] = friendId;
        js["msg"] = message;
        js["time"] = getCurrentTime();
        string buffer = js.dump();
        int len = send(clientfd, buffer.c_str(),strlen(buffer.c_str()) + 1, 0);
        if(len == -1) {
            cerr << "send chat msg error" << endl;
        }
    }
}

void creategroup(int clientfd,string str) {
    int idx = str.find(":");
}

void addgroup(int clientfd, string str) {

}

void groupchat(int clientfd, string str) {

}

void loginout(int, string) {

}