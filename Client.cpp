#include <sstream>
#include "Client.h"


Client::Client() {
    _fd = -1;
    _authenticated = false;
    _buffer = "";
}

int Client::getFd() {
    return _fd;
}

void Client::setFd(int fd) {
    _fd = fd;
}

void Client::setIpAddr(std::string addr) {
    _ipAddr = addr;
}

/* The get_next_line of ft_irc */
std::vector<std::string> extractCommands(std::string& commandBuffer) {
    std::vector<std::string> commands;

    size_t newlinePos = commandBuffer.find('\n');

    while (newlinePos != std::string::npos) {
        std::string command = commandBuffer.substr(0, newlinePos);
        commandBuffer.erase(0, newlinePos + 1);
        commands.push_back(command);
        newlinePos = commandBuffer.find('\n');
    }
    return commands;
}

bool startswith(const std::string& str, const std::string& needle) {
    if (str.length() < needle.length())
        return false;
    for (size_t i = 0; i < needle.length(); ++i) {
        if (str[i] != needle[i])
            return false;
    }
    return true;
}

std::vector<std::string> get_args(const std::string& command) {
    std::string commandCopy = command;
    size_t pos = 0;
    while ((pos = commandCopy.find('\r', pos)) != std::string::npos) {
        commandCopy[pos] = '\0';
    }

    std::vector<std::string> args;
    std::istringstream iss(commandCopy);
    std::string arg;

    while (iss >> arg) {
        /* the c_str is a horrendously ugly hack to remove excess \0 from the string */
        args.push_back(std::string(arg.c_str()));
    }

    return args;
}

void Client::setNick(const std::string &nick) {
    /* validate nick here */

    _nick = nick;
}

void Client::setUser(const std::string &user) {
    /* validate user here */

    _user = user;
}

void Client::processBuffer(const char *newBuff, Server &server) {
    _buffer += std::string(newBuff);
    std::vector<std::string> commands = extractCommands(_buffer);
    std::vector<std::string>::iterator it;
    for (it = commands.begin(); it != commands.end(); ++it) {
        std::vector<std::string> args = get_args(*it);
        if (args.empty()) {
            continue;
        }
        std::cout << "Command: " << args[0] << std::endl;
        if (args.size() > 1) {
            std::cout << "args[1]: " << args[1] << std::endl;
        }
        if (args[0] == "PASS" && args.size() > 1 && server.checkPassword(args[1])) {
            _authenticated = true;
        }
        if (!_authenticated) {
            std::cout << "Not authed, skipping command" << std::endl;
            continue;
        }
        if (args[0] == "NICK" && args.size() > 1) {
            setNick(args[1]);
        }
        if (args[0] == "USER" && args.size() > 1) {
            setUser(args[1]);
        }
        if (_user.empty() || _nick.empty()) {
            std::cout << "User or nick empty, skipping command" << std::endl;
            continue;
        }


    }
    std::cout << "Remaining in buffer: " << _buffer << std::endl;
}
