#include "Channel.h"

std::string Channel::getName() {
    return _name;
}

Channel::Channel() {

}

Channel::Channel(const std::string &name) {
    _name = name;
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
void Channel::addMode(const std::string &mode, std::vector<std::string> args, Server &server) {
    if (mode.empty()) {
        return;
    }
    char c_mode = mode.c_str()[0];
    _modes.insert(c_mode);
    if (c_mode == 'k') {
        // get password from args and set it
    } else if (c_mode == 'o') {

    } else if (c_mode == 'l') {

    }
}

bool Channel::hasMode(const char &mode) {
    return _modes.find(mode) != _modes.end();
}

void Channel::removeMode(const std::string &mode, Server &server) {

}

bool Channel::fdIsOp(int fd) {
    for (size_t i = 0; i < _opFds.size(); ++i) {
        if (_opFds[i] == fd) {
            return true;
        }
    }
    return false;
}

void Channel::op(int fd, Server &server) {

}

void Channel::deop(int fd, Server &server) {

}

void Channel::setTopic(const std::string &topic, Server &server) {
    _topic = topic;
}

void Channel::join(int fd, Server &server) {
    if (server.fdExists(fd)) {
        Client c = server.getClientByFd(fd);
        server.sendToAll(RPL_JOIN(c.getNick(), c.getUser(), c.getHost(), _name));

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
                if (fdIsOp(_clientFds[i])) {
                    users += "@";
                }
                users += c.getNick();
            }
        }
        c.reply(RPL_NAMREPLY(c.getNick(), _name, users));
        c.reply(RPL_ENDOFNAMES(c.getNick(), _name));
        //std::cout << "pushing back fd to "
        _clientFds.push_back(fd);
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


