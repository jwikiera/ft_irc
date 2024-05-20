#ifndef FT_IRC_CLIENT_H
#define FT_IRC_CLIENT_H

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <sstream>
#include "Server.h"
#include "responses.h"
#include "Util.h"

class Server;

//typedef void (Client::*MemberFunctionPtr)();


class Client {
private:
    int _fd;
    std::string _ipAddr;
    bool _authenticated; // true if has provided correct password
    std::string _buffer;
    std::string _pwd;
    std::string _nick;
    std::string _user;
    std::string _realname;
    std::string _servername;
    std::string _hostname;

    void setNick(const std::string &nick);
    void setUser(const std::string &user);
    void setRealName(const std::string &rname);
    void setHost(const std::string &host);
    void reg(Server &server);

public:
    typedef void (Client::*MemberFunctionPtr)(std::string &command, std::vector<std::string> &args, Server &server);
    static std::map<std::string, MemberFunctionPtr> createHandlerMap();
    static std::map<std::string, MemberFunctionPtr> functionMap;
    bool handleCommand(std::string &command, std::vector<std::string> &args, Server &server);

    Client();

    void reply(std::string rep) const;

    int getFd() const;
    std::string getNick();
    std::string getUser();
    std::string getRealName();
    std::string getHost();
    void setFd(int fd);
    void setIpAddr(const std::string &addr);
    void processBuffer(const char *newBuff, Server &server);

    void handleCommandNick(std::string &command, std::vector<std::string> &args, Server &server);

};

#endif //FT_IRC_CLIENT_H
