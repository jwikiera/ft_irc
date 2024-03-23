#include "Server.h"

bool Server::_gotSig = false;

Server::Server() {

}

Server::Server(char *portArg, char *pwdArg) {
    std::cout << "Initializing server...\n";
    _port = getPortFromArg(portArg);
    _password = std::string(pwdArg);
}

int Server::getPortFromArg(char *arg) {
    char *bak = arg;
    while (*arg) {
        if (!std::isdigit(*arg)) {
            return (-1);
        }
        arg ++;
    }
    return std::atoi(bak);
}

void Server::run() {
    registerSignals();
    while (!Server::_gotSig) {

    }
    std::cout << "Goodbye ~" << std::endl;
}

void Server::SigHandler(int signum) {
    std::cout << std::endl << "Received signal " << signum << std::endl;
    Server::_gotSig = true;
}

void Server::registerSignals() {
    if (signal(SIGINT, SigHandler) == SIG_ERR) {
        std::cerr << "Failed to bind signal SIGINT" << std::endl;
        exit(1);
    }
    if (signal(SIGQUIT, SigHandler) == SIG_ERR) {
        std::cerr << "Failed to bind signal SIGQUIT" << std::endl;
        exit(1);
    }
}
