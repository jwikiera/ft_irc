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

void Channel::addMode(const char &mode, Server &server) {

}

bool Channel::hasMode(const char &mode) {
    return false;
}

void Channel::removeMode(const char &mode, Server &server) {

}

bool Channel::fd_is_op(int fd) {
    return false;
}

void Channel::op(int fd, Server &server) {

}

void Channel::deop(int fd, Server &server) {

}

void Channel::setTopic(const std::string &topic, Server &server) {
    _topic = topic;
}
