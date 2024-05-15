#include "Client.h"


Client::Client() {
    _fd = -1;
    _authenticated = false;
    _buffer = "";
}

int Client::getFd() const {
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

std::vector<std::string> getArgs(const std::string& command) {
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

static void printArgs(const std::vector<std::string> &args) {
    std::cout << "Args:\n";
    for (size_t i = 0; i < args.size(); ++i) {
        std::cout << (i + 1) << ": " << args[i];
        if (i < args.size() - 1) {
            std::cout << "\n";
        } else {
            std::cout << std::endl;
        }
    }
}

void Client::processBuffer(const char *newBuff, Server &server) {
    _buffer += std::string(newBuff);
    std::vector<std::string> commands = extractCommands(_buffer);
    std::vector<std::string>::iterator it;
    for (it = commands.begin(); it != commands.end(); ++it) {
        std::vector<std::string> args = getArgs(*it);
        if (args.empty()) {
            continue;
        }
        printArgs(args);
        if (args[0] == "PING") {
            reply(RPL_PING);
            continue;
        }
        if (args[0] == "PASS" && args.size() > 1) {
            _pwd = args[1];
        }
        if (args[0] == "NICK" && args.size() > 1) {
            setNick(args[1]);
            if (!_authenticated) {
                reg(server);
            }
            continue;
        }
        /* we ignore second username */
        if (args[0] == "USER" && args.size() > 4) {
            if (_authenticated) {
                reply(ERR_ALREADYREGISTERED);
                continue;
            }
            setUser(args[1]);
            if (startswith(args[4], ":")) {
                setRealName(args[4].substr(1));
            }
            reg(server);
            continue;
        }
        if (args[0] == "CAP" && args.size() > 2 && args[1] == "LS" && args[2] == "302") {
            reply(RPL_CAP);
            continue;
        }
        if (args[0] == "CAP" && args.size() > 1 && args[1] == "REQ") {
            reply(RPL_CAP_REQ);
            continue;
        }
        if (args[0] == "CAP" && args.size() > 1 && args[1] == "END") {
            continue;
        }
        if (args[0] == "JOIN") {
            //std::cout << "Joinerino " << args[1] << " args len: " << args.size() << std::endl;
            if (args.size() == 2) {
                if (server.channelExists(args[1])) {

                } else {
                    server.createChannel(args[1]);
                }

            } else {
                reply(ERR_NEEDMOREPARAMS(std::string("JOIN")));
            }
            continue;
        }
        if (!_authenticated) {
            std::cout << "Not registered, skipping command" << std::endl;
            reply(ERR_NOTREGISTERED);
            continue;
        }
        /* — i : Définir/supprimer le canal sur invitation uniquement
            — t : Définir/supprimer les restrictions de la commande TOPIC pour les opérateurs de canaux
            — k : Définir/supprimer la clé du canal (mot de passe)
            — o : Donner/retirer le privilège de l’opérateur de canal
            — l : Définir/supprimer la limite d’utilisateurs pour le canal */
        if (args[0] == "MODE") {
            if (args.size() > 1) {
                /* we ignore user modes */
                if (startswith(args[1], "#")) {
                    std::string name = Util::removeFirstChar(args[1]);
                    if (server.channelExists(name)) {
                        std::string mode = args[2].substr(1, 2);
                        Channel &chan = server.getChannelByName(name);
                        if (!chan.hasFd(_fd)) {
                            reply(ERR_USERNOTINCHANNEL(_nick, chan.getName()));
                            continue;
                        }
                        if (startswith(args[2], "+")) {
                            chan.addMode(mode, args, server);
                        } else if (startswith(args[2], "-")) {
                            chan.removeMode(mode, server);
                        } else {
                            std::string modes = "+";
                            std::set<char>::iterator it_;
                            for (it_ = chan.getModes().begin(); it_ != chan.getModes().end(); ++it) {
                                modes += *it;
                            }
                            reply(RPL_CHANNEL_MODES(_nick, chan.getName(), modes));
                        }
                    } else {
                        reply(ERR_NOSUCHCHANNEL(name));
                    }
                } else {
                    reply(ERR_UNKNOWNMODE(std::string()));
                }
            } else {
                reply(ERR_NEEDMOREPARAMS(std::string("MODE")));
            }
            continue;
        }
        if (args[0] == "OPER") {
            if (args.size() > 2) {
                if (args[2] != "1234") {
                    reply(ERR_PASSWDMISMATCH);
                } else {
                    server.sendToAll(RPL_MODE_OPER(_nick));
                    reply(RPL_OPER(_nick));
                }
            } else {
                reply(ERR_NEEDMOREPARAMS(std::string("OPER")));
            }
            continue;
        }

    }

    std::cout << "Remaining in buffer: " << _buffer << std::endl;
}

void Client::reg(Server &server) {
    if (_user.empty() || _nick.empty()) {
        return;
    }
    if (!server.checkPassword(_pwd)) {
        reply(ERR_PASSWDMISMATCH);
    }
    _authenticated = true;
    reply(RPL_WELCOME(_nick));
    reply(RPL_MOTD_MISSING);
}

void Client::reply(std::string rep) const {
    rep = rep.substr(0, 510);
    std::cout << "attempting to reply with " << rep << " to " << _fd <<std::endl;
    rep += "\r\n";
    ssize_t sentBytes = send(_fd, rep.c_str(), rep.length(), 0);
    if (sentBytes < 0) {
        std::cerr << "Error sending reply to client <" << _fd << ">" << std::endl;
    }
}

std::string Client::getNick() {
    return _nick;
}

std::string Client::getUser() {
    return _user;
}

void Client::setRealName(const std::string &rname) {
    _realname = rname;
}

std::string Client::getRealName() {
    return _realname;
}
