#ifndef FT_IRC_SERVER_H
#define FT_IRC_SERVER_H

#include <cctype>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>

class Server {
private:
    Server();

    int _port;
    std::string _password;

    /* Stop server when we get a ctrl+c or ctrl+\ */
    static bool _gotSig;

    void registerSignals();

public:
    Server(char *portArg, char *pwdArg);

    static int getPortFromArg(char *arg);
    static void SigHandler(int signum);
    void run();
};


#endif //FT_IRC_SERVER_H
