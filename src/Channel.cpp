#include "Channel.h"

std::string Channel::getName() {
    return _name;
}

Channel::Channel() {
    _userLimit = -1;
}

Channel::Channel(const std::string &name) {
    _name = name;
    _userLimit = -1;
}

void Channel::setName(const std::string &name) {
    _name = name;
}

const std::string &Channel::getTopic() const {
    return _topic;
}

/* — i : Définir/supprimer le canal sur invitation uniquement
— t : Définir/supprimer les restrictions de la commande TOPIC pour les opérateurs de canaux
— k : Définir/supprimer la clé du canal (mot de passe)
— o : Donner/retirer le privilège de l’opérateur de canal
— l : Définir/supprimer la limite d’utilisateurs pour le canal */
void Channel::addMode(const std::string &mode, Client &who, std::vector<std::string> args, Server &server) {
    if (!fdIsOp(who.getFd(), server)) {
        who.reply(ERR_CHANOPRIVSNEEDED(_name, who.getNick()));
        return;
    }
    if (mode.empty()) {
        return;
    }
    char c_mode = mode.c_str()[0];

    if (c_mode == 'k') {
        if (args.size() > 3) {
            _password = args[3];
        }
    } else if (c_mode == 'o') {
        if (args.size() > 3) {
            if (!server.nickExists(args[3])) {
                who.reply(ERR_NOSUCHNICK(who.getNick(), args[3]));
            } else {
                _opFds.push_back(server.getClientByNick(args[3]).getFd());
            }
        }
    } else if (c_mode == 'l') {
        if (args.size() > 3) {
            try {
                _userLimit = (int)std::strtol(args[3].c_str(), NULL, 10);
            } catch (const std::exception& e) {
                (void) e;
            }
        }
    } else if (c_mode != 't' && c_mode != 'i') {
        who.reply(ERR_UNKNOWN_CHANNEL_MODE(mode, _name));
        return;
    }
    _modes.insert(c_mode);
    if (args.size() > 3) {
        sendToAll(RPL_MODE(who.getNick(), who.getUser(), who.getHost(), _name, "+" + mode, args[3]), server);
    } else {
        sendToAll(RPL_MODE(who.getNick(), who.getUser(), who.getHost(), _name, "+" + mode, ""), server);
    }
}

bool Channel::hasMode(const char &mode) {
    return _modes.find(mode) != _modes.end();
}

void Channel::removeMode(const std::string &mode, Client &who, std::vector<std::string> args, Server &server) {
    if (!fdIsOp(who.getFd(), server)) {
        who.reply(ERR_CHANOPRIVSNEEDED(_name, who.getNick()));
        return;
    }
    if (mode.empty()) {
        return;
    }
    char c_mode = mode.c_str()[0];

    if (c_mode == 'k') {
        _password = "";
    } else if (c_mode == 'o') {
        if (args.size() > 3) {
            if (!server.nickExists(args[3])) {
                who.reply(ERR_NOSUCHNICK(who.getNick(), args[3]));
            } else {
                removeOp(server.getClientByNick(args[3]).getFd());
            }
        }
    } else if (c_mode == 'l') {
        if (args.size() > 3) {
            _userLimit = -1;
        }
    } else if (c_mode != 't' && c_mode != 'i') {
        who.reply(ERR_UNKNOWN_CHANNEL_MODE(mode, _name));
        return;
    }
    _modes.erase(c_mode);

    if (args.size() > 3) {
        sendToAll(RPL_MODE(who.getNick(), who.getUser(), who.getHost(), _name, "-" + mode, args[3]), server);
    } else {
        sendToAll(RPL_MODE(who.getNick(), who.getUser(), who.getHost(), _name, "-" + mode, ""), server);
    }
}

bool Channel::fdIsOp(int fd, Server &server) {
    if (server.fdIsGlobalOp(fd)) {
        return true;
    }
    for (size_t i = 0; i < _opFds.size(); ++i) {
        if (_opFds[i] == fd) {
            return true;
        }
    }
    return false;
}

void Channel::setTopic(const std::string &topic, Client &who, Server &server) {
    _topic = topic;
    sendToAll(RPL_TOPIC(who.getNick(), who.getUser(), who.getHost(), _name, topic), server);
}

void Channel::join(int fd, Server &server) {
    if (server.fdExists(fd)) {
        Client &c = server.getClientByFd(fd);
        c.reply(RPL_JOIN(c.getNick(), c.getUser(), c.getHost(), _name));
        sendToAll(RPL_JOIN(c.getNick(), c.getUser(), c.getHost(), _name), server);

        std::string users = "";
        if (_clientFds.empty()) {
            _opFds.push_back(fd);
        }
        _clientFds.push_back(fd);
        std::cout << "pushing fd " << fd << " to fds..., size is now " << _clientFds.size() << std::endl;

        for (size_t i = 0; i < _clientFds.size(); ++i) {
            if (server.fdExists(_clientFds[i])) {
                if (i > 0) {
                    users += " ";
                }
                if (fdIsOp(_clientFds[i], server)) {
                    users += "@";
                }
                users += server.getClientByFd(_clientFds[i]).getNick();
            }
        }
        c.reply(SEND_TOPIC(c.getNick(), _name, _topic));
        c.reply(RPL_NAMREPLY(c.getNick(), _name, users));
        c.reply(RPL_ENDOFNAMES(c.getNick(), _name));
    }
}

void Channel::part(int fd, Server &server) {

}

bool Channel::hasFd(int fd) {
    for (size_t i = 0; i < _clientFds.size(); ++i) {
        if (_clientFds[i] == fd) {
            return true;
        }
    }
    return false;
}

std::set<char> &Channel::getModes() {
    return _modes;
}

std::vector<int> Channel::getClientFds() {
    return _clientFds;
}

std::string Channel::getPassword() {
    return _password;
}

int Channel::getUserLimit() const {
    return _userLimit;
}

void Channel::removeOp(int fd) {
    _opFds.erase(std::remove(_opFds.begin(), _opFds.end(), fd), _opFds.end());
}

void Channel::removeClient(int fd) {
    _clientFds.erase(std::remove(_clientFds.begin(), _clientFds.end(), fd), _clientFds.end());
}

void Channel::removeFd(int fd) {
    removeOp(fd);
    removeClient(fd);
    deinviteFd(fd);
}

void Channel::inviteFd(int fd) {
    _invitedFds.push_back(fd);
}

void Channel::deinviteFd(int fd) {
    _invitedFds.erase(std::remove(_invitedFds.begin(), _invitedFds.end(), fd), _invitedFds.end());
}

bool Channel::fdInvited(int fd) {
    for (size_t i = 0; i < _invitedFds.size(); ++i) {
        if (_invitedFds[i] == fd) {
            return true;
        }
    }
    return false;
}

void Channel::sendToAll(const std::string &rep, Server &server) {
    std::string rep_ = rep.substr(0, 510);
    std::cout << "attempting to reply with " << rep_ << " to all in channel " << _name << std::endl;
    //rep_ += "\r\n";
    for (size_t i = 0; i < _clientFds.size(); i ++) {
        server.sendToClient(rep_, _clientFds[i]);
    }
}
