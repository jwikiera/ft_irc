#include "Server.h"

bool Server::_gotSig = false;

Server::Server() {
    _serverSocketFd = -1;
    _port = -1;
}

void Server::init(char *portArg, char *pwdArg) {
    std::cout << "Initializing server...\n";
    registerSignals();
    _port = getPortFromArg(portArg);
    _password = std::string(pwdArg);
    _serverSocketFd = -1;
    createServerSocket();
    std::cout << GRE << "Server <" << _serverSocketFd << "> Connected" << WHI << std::endl;
    std::cout << "Waiting to accept a connection...\n";

    while (!Server::_gotSig) {
        /* Waiting for event */
        /* poll basically tells us if i/o is possible on any fd inside _fds, otherwise it blocks */
        if ((poll(&_fds[0],_fds.size(),-1) == -1) && !Server::_gotSig) {
            throw (std::runtime_error("poll() failed"));
        }

        for (size_t i = 0; i < _fds.size(); i++) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _serverSocketFd) { /* here we check if we got something on the server socket, and it's a new client */
                    acceptClient();
                } else { /* otherwise, it's data which came on a socket already associated with a client */
                    receiveData(_fds[i].fd);
                }
            }
        }
    }
    closeFds();
    std::cout << "Goodbye ~" << std::endl;
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

/* yeet client both from poll and internal client vector */
void Server::removeClient(int fd) {
    std::cout << "Removing client " << fd << std::endl;
    for (size_t i = 0; i < _fds.size(); i++){
        if (_fds[i].fd == fd) {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }
    for (size_t i = 0; i < _clients.size(); i++){
        if (_clients[i].getFd() == fd) {
            _clients.erase(_clients.begin() + i);
            break;
        }
    }
    for (size_t i = 0; i < _channels.size(); ++i) {
        _channels[i].removeFd(fd);
    }
    close(fd);
}

void Server::closeFds() {
    for (size_t i = 0; i < _clients.size(); i++){ //-> close all the clients
        std::cout << RED << "Client <" << _clients[i].getFd() << "> Disconnected" << WHI << std::endl;
        close(_clients[i].getFd());
    }
    if (_serverSocketFd != -1){ //-> close the server socket
        std::cout << RED << "Server <" << _serverSocketFd << "> Disconnected" << WHI << std::endl;
        close(_serverSocketFd);
    }
}

void Server::createServerSocket() {
    struct sockaddr_in addr;
    struct pollfd newPoll;
    addr.sin_family = AF_INET; /* tells we're using ipv4 */
    addr.sin_port = htons(this->_port); /* convert the port to network byte order (big endian) */
    addr.sin_addr.s_addr = INADDR_ANY; /* lets clients connect to any address used for the local machine */

    _serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(_serverSocketFd == -1) {
        throw (std::runtime_error("Failed to create socket"));
    }

    /* setsockopt: Handy debugging trick that lets
   * us rerun the server immediately after we kill it;
   * otherwise we have to wait about 20 secs.
   * Eliminates "ERROR on binding: Address already in use" error.
    * source: https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/selectserver.c
   */
    int optval = 1;
    if (setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        throw (std::runtime_error("Failed to set option (SO_REUSEADDR) on socket"));
    }

    /* Config socket as non-blocking: TLDR: operations such as read() and write() on the socket will return immediately */
    if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) == -1) {
        throw (std::runtime_error("Failed to set option (O_NONBLOCK) on socket"));
    }

    if (bind(_serverSocketFd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        throw (std::runtime_error("Failed to bind socket"));
    }

    if (listen(_serverSocketFd, SOMAXCONN) == -1) {
        throw (std::runtime_error("listen() failed :("));
    }

    newPoll.fd = _serverSocketFd; // addr the server socket to the pollfd
    newPoll.events = POLLIN; /* POLLIN to read data */
    newPoll.revents = 0; // set the revents to 0
    _fds.push_back(newPoll); // add the server socket to the pollfd
}

void Server::acceptClient() {
    Client cli;
    struct sockaddr_in cliAddress;
    struct pollfd newPoll;
    socklen_t len = sizeof(cliAddress);

    int incofd = accept(_serverSocketFd, (sockaddr *)&(cliAddress), &len); // accept the new client
    if (incofd == -1) {
        std::cout << "accept() failed" << std::endl; return;
    }

    /* set the socket option (O_NONBLOCK) for non-blocking socket */
    if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) {
        std::cout << "fcntl() failed" << std::endl; return;
    }

    newPoll.fd = incofd;
    newPoll.events = POLLIN;
    newPoll.revents = 0;

    cli.setFd(incofd);
    cli.setIpAddr(inet_ntoa((cliAddress.sin_addr)));
    _clients.push_back(cli);
    _fds.push_back(newPoll);

    std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::receiveData(int fd) {
    char buff[1024];
    bzero(buff, sizeof(buff));

    /* receives data into buffer */
    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);

    /* check if client disconnected */
    if (bytes <= 0) {
        std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
        removeClient(fd);
        //close(fd);
    } else {
        buff[bytes] = '\0';
        std::cout << YEL << "Client <" << fd << "> Data:\n" << WHI << "`" << buff << "`" << std::endl;

        /* process data */
        if (fdExists(fd)) {
            getClientByFd(fd).processBuffer(buff, *this);
        }
    }
}

bool Server::checkPassword(const std::string &pwd) {
    return pwd == _password;
}

Client &Server::getClientByNick(const std::string& nick) {
    for (size_t i = 0; i < _clients.size(); i++){
        if (_clients[i].getNick() == nick) {
            return (_clients[i]);
        }
    }
    throw std::runtime_error("Client not found (by nick)");
}

bool Server::nickExists(const std::string &nick) {
    for (size_t i = 0; i < _clients.size(); i++){
        if (_clients[i].getNick() == nick) {
            return (true);
        }
    }
    return (false);
}

bool Server::channelExists(const std::string &name) {
    for (size_t i = 0; i < _channels.size(); i++){
        if (_channels[i].getName() == name) {
            return (true);
        }
    }
    return (false);
}

Channel &Server::getChannelByName(const std::string &name) {
    for (size_t i = 0; i < _channels.size(); i++){
        if (_channels[i].getName() == name) {
            return (_channels[i]);
        }
    }
    throw std::runtime_error("Channel not found");
}

Channel &Server::createChannel(const std::string &name) {
    Channel res(name);
    _channels.push_back(res);
    return (_channels[_channels.size() - 1]);
}

void Server::removeChannel(const std::string &name) {
    for (size_t i = 0; i < _channels.size(); i++){
        if (_channels[i].getName() == name) {
            _channels.erase(_channels.begin() + i);
            break;
        }
    }
}

void Server::sendToClient(const std::string &rep, int fd) {
    std::string rep_ = rep.substr(0, 510);
    std::cout << "attempting to reply with " << rep_ << " to " << fd <<std::endl;
    rep_ += "\r\n";
    ssize_t sentBytes = send(fd, rep_.c_str(), rep_.length(), 0);
    if (sentBytes < 0) {
        std::cerr << "Error sending reply to client <" << fd << ">" << std::endl;
    }
}

void Server::sendToAll(const std::string &rep) {
    std::string rep_ = rep.substr(0, 510);
    std::cout << "attempting to reply with " << rep_ << " to all" << std::endl;
    rep_ += "\r\n";
    for (size_t i = 0; i < _clients.size(); i ++) {
        ssize_t sentBytes = send(_clients[i].getFd(), rep_.c_str(), rep_.length(), 0);
        if (sentBytes < 0) {
            std::cerr << "Error sending reply to client <" << _clients[i].getFd() << ">" << std::endl;
        }
    }
}

bool Server::fdExists(int fd) {
    for (size_t i = 0; i < _clients.size(); i++){
        if (_clients[i].getFd() == fd) {
            return (true);
        }
    }
    return (false);
}

Client &Server::getClientByFd(int fd) {
    for (size_t i = 0; i < _clients.size(); i++){
        if (_clients[i].getFd() == fd) {
            return (_clients[i]);
        }
    }
    throw std::runtime_error("Client not found (by fd)");
}

void Server::op(int fd) {
    _globalOpFds.push_back(fd);
}

bool Server::fdIsGlobalOp(int fd) {
    for (size_t i = 0; i < _globalOpFds.size(); ++i) {
        if (_globalOpFds[i] == fd) {
            return true;
        }
    }
    return false;
}

std::vector<Channel> &Server::getChannels() {
    return _channels;
}

void Server::debugPrintFds() {
    std::string res = "Fds:";
    for (size_t i = 0; i < _fds.size(); ++i) {
        res += "`";
        res += Util::streamItoa(_fds[i].fd);
        res += "`, ";
    }
    std::cout << res << std::endl;
}

void Server::debugPrintChannels() {
    std::string res = "Channels:";
    for (size_t i = 0; i < _channels.size(); ++i) {
        res += "`";
        res += _channels[i].getName();
        res += "`, ";
    }
    std::cout << res << std::endl;
}
