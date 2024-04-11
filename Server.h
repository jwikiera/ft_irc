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
#include "Client.h"

/* colors */
#define RED "\033[0;31m"
#define WHI "\033[0;37m"
#define GRE "\033[0;32m"
#define YEL "\033[0;33m"

class Client;

class Server {
private:
    int _port;
    int _serverSocketFd;
    std::string _password;
    std::vector<Client> _clients;
    std::vector<struct pollfd> _fds;

    /* Stop server when we get a ctrl+c or ctrl+\ */
    static bool _gotSig;

    void registerSignals();
    Client &getClientByFd(int fd);

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

    bool checkPassword(std::string pwd);
};


#endif //FT_IRC_SERVER_H
