#ifndef FT_IRC_CLIENT_H
#define FT_IRC_CLIENT_H

#include <string>
#include <vector>
#include <iostream>
#include "Server.h"

class Server;

class Client {
private:
    int _fd;
    std::string _ipAddr;
    bool _authenticated; // true if has provided correct password
    std::string _buffer;
    std::string _pwd;
    std::string _nick;
    std::string _user;

    void setNick(const std::string &nick);
    void setUser(const std::string &user);
    void reg(Server &server);

public:
    Client();
    int getFd();
    void setFd(int fd);
    void setIpAddr(const std::string& addr);
    void processBuffer(const char *newBuff, Server &server);
};


#endif //FT_IRC_CLIENT_H
