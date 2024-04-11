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

public:
    Client();
    int getFd();
    void setFd(int fd);
    void setIpAddr(std::string addr);
    void processBuffer(const char *newBuff, Server &server);
};


#endif //FT_IRC_CLIENT_H
