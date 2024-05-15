#ifndef FT_IRC_CHANNEL_H
#define FT_IRC_CHANNEL_H

#include <vector>
#include <string>
#include <set>
#include "Server.h"


class Server;

class Channel {
private:
    Channel();

    std::vector<int> _clientFds;
    std::vector<int> _opFds;
    std::set<char> _modes;
    std::string _name;
    std::string _topic;
    std::string _password;
    int _userLimit;

public:
    void setName(const std::string &name);
    std::set<char> &getModes();
    std::vector<int> getClientFds();

    const std::string &getTopic() const;

    void setTopic(const std::string &topic, Server &server);

    Channel(const std::string &name);
    std::string getName();

    void addMode(const std::string &mode, std::vector<std::string> args, Server &server);
    bool hasMode(const char &mode);
    void removeMode(const std::string &mode, Server &server);

    bool hasFd(int fd);
    bool fdIsOp(int fd);
    void op(int fd, Server &server);
    void deop(int fd, Server &server);

    void join(int fd, Server &server);
    void part(int fd, Server &server);
};


#endif //FT_IRC_CHANNEL_H
