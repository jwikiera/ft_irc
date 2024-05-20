#ifndef FT_IRC_CHANNEL_H
#define FT_IRC_CHANNEL_H

#include <algorithm>
#include <vector>
#include <string>
#include <set>
#include "Server.h"


class Server;
class Client;

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
    std::vector<int> _invitedFds;

public:
    void setName(const std::string &name);
    std::set<char> &getModes();
    std::vector<int> getClientFds();
    std::string getPassword();
    int getUserLimit() const;

    const std::string &getTopic() const;

    void setTopic(const std::string &topic, Client &who, Server &server);

    Channel(const std::string &name);
    std::string getName();

    void addMode(const std::string &mode, Client &who, std::vector<std::string> args, Server &server);
    bool hasMode(const char &mode);
    void removeMode(const std::string &mode, Client &who, std::vector<std::string> args, Server &server);

    bool hasFd(int fd);
    bool fdIsOp(int fd, Server &server);

    void join(int fd, Server &server);
    void part(int fd, Server &server);

    void removeOp(int fd);
    void removeClient(int fd);
    void removeFd(int fd);

    void inviteFd(int fd);
    void deinviteFd(int fd);
    bool fdInvited(int fd);

    void sendToAll(const std::string &rep, Server &server);
};


#endif //FT_IRC_CHANNEL_H
