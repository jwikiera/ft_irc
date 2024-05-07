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

void Client::setIpAddr(const std::string& addr) {
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
        args.push_back(Util::trimSpace(std::string(arg.c_str())));
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
        if (args[0] == "PING") {
            reply(RPL_PING, _fd);
            continue;
        }
        if (args[0] == "PASS" && args.size() > 1) {
            _pwd = args[1];
        }
        if (args[0] == "NICK" && args.size() > 1) {
            setNick(args[1]);
            if (!_authenticated) {
                reg(server, _fd);
            }
            continue;
        }
        /* we ignore second username and real name */
        if (args[0] == "USER" && args.size() > 1) {
            if (_authenticated) {
                reply(ERR_ALREADYREGISTERED, _fd);
                continue;
            }
            setUser(args[1]);
            reg(server, _fd);
            continue;
        }
        if (args[0] == "CAP" && args.size() > 2 && args[1] == "LS" && args[2] == "302") {
            reply(RPL_CAP, _fd);
            continue;
        }
        if (args[0] == "CAP" && args.size() > 1 && args[1] == "REQ") {
            reply(RPL_CAP_REQ, _fd);
            continue;
        }
        if (args[0] == "CAP" && args.size() > 1 && args[1] == "END") {
            continue;
        }
        if (args[0] == "JOIN") {
            //std::cout << "Joinerino " << args[1] << " args len: " << args.size() << std::endl;
            if (args.size() == 2) {
                if (args[1] == ":") { // what???
                    reply(ERR_NOTREGISTERED, _fd);

                }

            } else {

            }
            continue;
        }
        if (!_authenticated) {
            std::cout << "Not registered, skipping command" << std::endl;
            reply(ERR_NOTREGISTERED, _fd);
            continue;
        }
        /* — i : Définir/supprimer le canal sur invitation uniquement
            — t : Définir/supprimer les restrictions de la commande TOPIC pour les opérateurs de canaux
            — k : Définir/supprimer la clé du canal (mot de passe)
            — o : Donner/retirer le privilège de l’opérateur de canal
            — l : Définir/supprimer la limite d’utilisateurs pour le canal */
        if (args[0] == "MODE") {
            if (args.size() > 3) {
                if (startswith(args[1], "#")) {
                    std::string name = Util::removeFirstChar(args[1]);
                    if (server.channelExists(name)) {
                        if (startswith(args[2], "+")) {

                        } else if (startswith(args[2], "-")) {

                        } else {
                            reply(ERR_UNKNOWNMODE(std::string()), _fd);
                        }
                    } else {
                        reply(ERR_NOSUCHCHANNEL(name), _fd);
                    }
                } else {
                    reply(ERR_UNKNOWNMODE(std::string()), _fd);
                }
            } else {
                reply(ERR_NEEDMOREPARAMS(std::string("MODE")), _fd);
            }
            continue;
        }

    }
    std::cout << "Remaining in buffer: " << _buffer << std::endl;
}

void Client::reg(Server &server, int client_fd) {
    if (_user.empty() || _nick.empty()) {
        return;
    }
    if (!server.checkPassword(_pwd)) {
        reply(ERR_PASSWDMISMATCH, client_fd);
    }
    _authenticated = true;
    reply(RPL_WELCOME, client_fd);
    reply(RPL_MOTD_MISSING, client_fd);
}

void Client::reply(std::string rep, int fd) {
    rep = rep.substr(0, 510);
    std::cout << "attempting to reply with " << rep << " to " << fd <<std::endl;
    rep += "\r\n";
    ssize_t sentBytes = send(fd, rep.c_str(), rep.length(), 0);
    if (sentBytes < 0) {
        // Handle send error
        std::cerr << "Error sending reply to client <" << fd << ">" << std::endl;
    }
}

std::string Client::getNick() {
    return _nick;
}
