#include <iostream>
#include <string>

#include "Server.h"

int main(int argc, char *argv[]) {
    std::cout << "Hello from ircserv!\n";
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <port> <password>\n";
        exit(1);
    }

    if (Server::getPortFromArg(argv[1]) == -1) {
        std::cout << "Invalid port!\n";
        exit(1);
    }

    Server server;

    //try {
        server.init(argv[1], argv[2]);
    /*} catch(const std::exception& e) {
        server.closeFds();
        std::cerr << e.what() << std::endl;
        return 1;
    }*/
    std::cout << "The Server Closed!" << std::endl;
    return 0;
}
