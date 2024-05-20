#ifndef FT_IRC_SERVER_H
#define FT_IRC_SERVER_H

#include <arpa/inet.h>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <vector>
#include <cstring>
#include <unistd.h>
#include "Client.h"
#include "Channel.h"

/* colors */
#define RED "\033[0;31m"
#define WHI "\033[0;37m"
#define GRE "\033[0;32m"
#define YEL "\033[0;33m"

class Client;
class Channel;

class Server {
private:
    int _port;
    int _serverSocketFd;
    std::string _password;
    std::vector<Client> _clients;
    std::vector<Channel> _channels;
    std::vector<struct pollfd> _fds;
    std::vector<int> _globalOpFds;

    /* Stop server when we get a ctrl+c or ctrl+\ */
    static bool _gotSig;

    void registerSignals();

public:
    Server();

    void init(char *portArg, char *pwdArg);
    void createServerSocket();
    void acceptClient();
    void closeFds();
    void removeClient(int fd);
    void receiveData(int fd);

    static int getPortFromArg(char *arg);
    static void SigHandler(int signum);

    bool checkPassword(const std::string &pwd);

    bool nickExists(const std::string &nick);
    Client &getClientByNick(const std::string &nick);
    bool fdExists(int fd);
    Client &getClientByFd(int fd);

    bool channelExists(const std::string &name);
    Channel &getChannelByName(const std::string &name);
    std::vector<Channel> &getChannels();

    Channel &createChannel(const std::string &name);
    void removeChannel(const std::string &name);

    // this might get moved to a command class or smth
    void kick(const std::string &channel, const std::string &nick);
    void invite(const std::string &channel, const std::string &nick);
    void setTopic(const std::string &channel);
    void getTopic(const std::string &channel);

    void sendToClient(const std::string &rep, int fd);
    void sendToAll(const std::string &rep);

    void op(int fd);
    bool fdIsGlobalOp(int fd);

    std::string hostname;
};


#endif //FT_IRC_SERVER_H
