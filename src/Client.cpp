#include "Client.h"


std::map<std::string, Client::MemberFunctionPtr> Client::functionMap = Client::createHandlerMap();

Client::Client() {
    _fd = -1;
    _authenticated = false;
    _buffer = "";
}

std::map<std::string, Client::MemberFunctionPtr> Client::createHandlerMap() {
    std::map<std::string, MemberFunctionPtr> map;
    map["NICK"] = &Client::handleCommandNick;
    map["PING"] = &Client::handleCommandPing;
    map["PASS"] = &Client::handleCommandPass;
    map["CAP"] = &Client::handleCommandCap;
    map["USER"] = &Client::handleCommandUser;
    map["JOIN"] = &Client::handleCommandJoin;
    map["MODE"] = &Client::handleCommandMode;
    map["OPER"] = &Client::handleCommandOper;
    map["WHO"] = &Client::handleCommandWho;
    map["PRIVMSG"] = &Client::handleCommandPrivmsg;
    map["TOPIC"] = &Client::handleCommandTopic;
    map["KICK"] = &Client::handleCommandKick;
    map["PART"] = &Client::handleCommandPart;
    map["QUIT"] = &Client::handleCommandQuit;
    map["MSG"] = &Client::handleCommandMsg;
    map["INVITE"] = &Client::handleCommandInvite;
    return map;
}

// Method to call a function from the map
bool Client::handleCommand(std::string &command, std::vector<std::string> &args, Server &server) {
    std::map<std::string, MemberFunctionPtr>::iterator it = functionMap.find(args[0]);
    if (it != functionMap.end()) {
        MemberFunctionPtr func = it->second;
        (this->*func)(command, args, server);  // Call the member function
    } else {
        std::cout << "Function not found: " << command << std::endl;
        return false;
    }
    return true;
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

static std::string extractMessage(const std::string& command) {
    size_t colon_pos = command.rfind(':');

    if (colon_pos == std::string::npos) {
        return ""; // No colon found, return empty string
    }

    std::string message = command.substr(colon_pos + 1);

    if (!message.empty() && message[message.size() - 1] == '\r') {
        message.erase(message.size() - 1);
    }

    return message;
}

static std::string channelNameFromArg(std::string arg) {
    if (arg.empty()) {
        return "";
    }
    if (startswith(arg, "#")) {
        return arg.substr(1);
    } else {
        return arg;
    }
}

void Client::handleCommandNick(std::string &command, std::vector<std::string> &args, Server &server)  {
    if (args.size() > 1) {
        if (server.nickExists(args[1])) {
            reply(ERR_NICKNAMEINUSE(_nick, args[1]));
            return;
        }
        std::string oldNick = _nick;
        setNick(args[1]);
        if (!_authenticated) {
            reg(server);
        }
        if (_authenticated) {
            server.sendToAll(RPL_NICK(oldNick, _user, _hostname, _nick));
        }
    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("NICK")));
    }
}

void Client::handleCommandPass(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    (void) server;
    if (args.size() > 1) {
        _pwd = args[1];
    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("PASS")));
    }
}

void Client::handleCommandPing(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    (void) args;
    (void) server;
    reply(RPL_PING);
}

void Client::handleCommandCap(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    (void) server;
    if (args[0] == "CAP" && args.size() > 2 && args[1] == "LS" && args[2] == "302") {
        reply(RPL_CAP);
        return;
    }
    if (args[0] == "CAP" && args.size() > 1 && args[1] == "REQ") {
        reply(RPL_CAP_REQ);
        return;
    }
}

void Client::handleCommandUser(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    if (args.size() > 4) {
        if (_authenticated) {
            reply(ERR_ALREADYREGISTERED);
            return;
        }
        setUser(args[1]);
        if (startswith(args[4], ":")) {
            setRealName(args[4].substr(1));
        }
        setHost(args[3]);
        reg(server);
    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("USER")));
    }
}

void Client::handleCommandJoin(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    if (args.size() > 1 && channelNameFromArg(args[1]).size() > 0) {
        if (!server.channelExists(channelNameFromArg(args[1]))) {
            server.createChannel(channelNameFromArg(args[1]));
        }
        Channel &chan = server.getChannelByName(channelNameFromArg(args[1]));
        if (chan.hasFd(_fd)) {
            reply(ERR_USERONCHANNEL(_nick, chan.getName()));
            return;
        }
        if (chan.hasMode('l') && chan.getUserLimit() > -1 && chan.getUserLimit() == (int)chan.getClientFds().size()) {
            reply(ERR_CHANNELISFULL(_nick, chan.getName()));
            return;
        }
        if (chan.hasMode('k') && !chan.getPassword().empty()) {
            if (args.size() < 3 || (args.size() > 2 && chan.getPassword() != args[2])) {
                reply(ERR_BADCHANNELKEY(_nick, chan.getName()));
                return;
            }
        }
        if (chan.hasMode('i')) {
            if (!chan.fdInvited(_fd)) {
                reply(RPL_MUST_BE_INVITED(_nick, chan.getName()));
                return;
            }
        }
        chan.join(_fd, server);
    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("JOIN")));
    }
}

/* — i : Définir/supprimer le canal sur invitation uniquement
            — t : Définir/supprimer les restrictions de la commande TOPIC pour les opérateurs de canaux
            — k : Définir/supprimer la clé du canal (mot de passe)
            — o : Donner/retirer le privilège de l’opérateur de canal
            — l : Définir/supprimer la limite d’utilisateurs pour le canal */
void Client::handleCommandMode(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    (void) args;
    (void) server;
    if (args.size() > 1) {

        std::string name = channelNameFromArg(args[1]);
        if (server.channelExists(name)) {
            Channel &chan = server.getChannelByName(name);
            if (args.size() > 2) {
                std::string mode = "";
                if (args[2].size() > 1) {
                    mode = args[2].substr(1, 2);
                }
                if (!chan.hasFd(_fd)) {
                    reply(ERR_USERNOTINCHANNEL(_nick, chan.getName()));
                    return;
                }
                if (startswith(args[2], "b")) {
                    reply(END_BAN(_nick, chan.getName()));
                } else if (startswith(args[2], "+")) {
                    chan.addMode(mode, *this, args, server);
                } else if (startswith(args[2], "-")) {
                    chan.removeMode(mode, *this, args, server);
                }
            } else {
                std::string modes = "+";
                std::set<char>::iterator it_;
                std::cout << "The size of chan modes is " << chan.getModes().size() << std::endl;
                for (it_ = chan.getModes().begin(); it_ != chan.getModes().end(); it_ ++) {
                    if (*it_ == 'k' && chan.getPassword().empty()) {
                        continue;
                    }
                    modes += *it_;
                }
                /* iterate over the mode string and append user limit and password if inside */
                for (size_t i = 0; i < modes.size(); ++i) {
                    if (modes[i] == 'l' && chan.getUserLimit() > 0) {
                        modes += " ";
                        modes += Util::streamItoa(chan.getUserLimit());
                    }
                    if (modes[i] == 'k' && !chan.getPassword().empty()) {
                        modes += " ";
                        modes += chan.getPassword();
                    }
                }
                reply(RPL_CHANNEL_MODES(_nick, chan.getName(), modes));
            }
        } else if (server.nickExists(args[1])) {
            if (args.size() > 2) {
                std::string mode = "";
                if (args[2].size() > 1) {
                    mode = args[2].substr(1, 2);
                }
                reply(RPL_UNKNOWN_USER_MODE(_nick, mode));
            } else {
                if (args[1] == _nick) {
                    std::string mode = "+";
                    if (server.fdIsGlobalOp(_fd)) {
                        mode += "o";
                    }
                    reply(RPL_NICK_MODES(_nick, mode));
                } else {
                    reply(RPL_CANT_MODE_OTHER(_nick));
                }
            }
        } else {
            reply(ERR_NOSUCHNICK(_nick, args[1]));
        }
    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("MODE")));
    }
}

void Client::handleCommandOper(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    if (args.size() > 2) {
        if (args[2] != "1234") {
            reply(ERR_PASSWDMISMATCH);
        } else {
            reply(RPL_MODE_OPER(_nick));
            reply(RPL_OPER(_nick));
            server.op(_fd);
        }
    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("OPER")));
    }
}

void Client::handleCommandWho(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    if (args.size() > 1) {
        if (!server.channelExists(channelNameFromArg(args[1]))) {
            reply(ERR_NOSUCHCHANNEL(args[1]));
            return;
        }
        Channel &chan = server.getChannelByName(channelNameFromArg(args[1]));
        std::cout << "Amount of users in channel for WHO: " << chan.getClientFds().size() << std::endl;
        for (size_t i = 0; i < chan.getClientFds().size(); i ++) {
            if (!server.fdExists(chan.getClientFds()[i])) {
                continue;
            }
            Client &client = server.getClientByFd(chan.getClientFds()[i]);
            std::string flags = "H";
            if (chan.fdIsOp(chan.getClientFds()[i], server)) {
                flags = "H@";
            }
            reply(RPL_WHO(_nick, chan.getName(), client.getUser(), client.getHost(), client.getNick(), flags, client.getRealName()));
        }
        reply(END_WHO(_nick, chan.getName()));
    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("WHO")));
    }
}

void Client::handleCommandPrivmsg(std::string &command, std::vector<std::string> &args, Server &server)  {
    if (args.size() > 2) {
        if (server.channelExists(channelNameFromArg(args[1]))) {
            Channel &chan = server.getChannelByName(channelNameFromArg(args[1]));
            if (!chan.hasFd(_fd)) {
                reply(ERR_USERNOTINCHANNEL(_nick, chan.getName()));
                return;
            }
            std::string message = extractMessage(command);
            for (size_t i = 0; i < chan.getClientFds().size(); i ++) {
                if (!server.fdExists(chan.getClientFds()[i])) {
                    continue;
                }
                Client &client = server.getClientByFd(chan.getClientFds()[i]);
                if (client.getFd() != _fd) {
                    client.reply(RPL_PRIVMSG(getNick(), getUser(), getHost(), chan.getName(), message));
                }
            }
        } else if (server.nickExists(args[1])) {
            if (!server.nickExists(args[1])) {
                reply(ERR_NOSUCHNICK(_nick, args[1]));
                return;
            }
            std::string message = extractMessage(command);
            Client &c = server.getClientByNick(args[1]);
            c.reply(PRIV_MSG_DM(_nick, _user, _hostname, c.getNick(), message));
        } else {
            reply(ERR_NOSUCHNICK(_nick, args[1]));
            return;
        }

    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("PRIVMSG")));
    }
}

void Client::handleCommandTopic(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    (void) args;
    (void) server;
    if (args.size() < 2) {
        reply(ERR_NEEDMOREPARAMS(std::string("TOPIC")));
        return;
    }

    if (!server.channelExists(channelNameFromArg(args[1]))) {
        reply(ERR_NOSUCHCHANNEL(args[1]));
        return;
    }

    Channel &chan = server.getChannelByName(channelNameFromArg(args[1]));
    if (args.size() > 2) {
        if (!chan.hasFd(_fd)) {
            reply(ERR_USERNOTINCHANNEL(_nick, chan.getName()));
            return;
        }
        if (chan.hasMode('t') && !chan.fdIsOp(_fd, server)) {
            reply(ERR_CHANOPRIVSNEEDED(chan.getName(), _nick));
            return;
        }
        std::string topic = extractMessage(command);
        chan.setTopic(topic, *this, server);
    } else {
        reply(RPL_TOPIC_COMMAND(_nick, chan.getName(), chan.getTopic()));
    }
}

void Client::handleCommandKick(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    if (args.size() > 2) {
        if (!server.channelExists(channelNameFromArg(args[1]))) {
            reply(ERR_NOSUCHCHANNEL(args[1]));
            return;
        }
        Channel &chan = server.getChannelByName(channelNameFromArg(args[1]));
        if (!chan.hasFd(_fd)) {
            reply(ERR_USERNOTINCHANNEL(_nick, chan.getName()));
            return;
        }
        if (!chan.fdIsOp(_fd, server)) {
            reply(ERR_CHANOPRIVSNEEDED(chan.getName(), _nick));
            return;
        }
        if (server.nickExists(args[2])) {
            Client &target = server.getClientByNick(args[2]);
            if (chan.hasFd(target.getFd())) {
                chan.sendToAll(RPL_KICK(_nick, _user, _hostname, chan.getName(), target.getNick()), server);
                chan.removeFd(target.getFd());
                if (chan.getClientFds().empty()) {
                    server.removeChannel(chan.getName());
                }
            } else {
                reply(ERR_NOSUCHNICK(_nick, args[2]));
                return;
            }
        } else {
            reply(ERR_NOSUCHNICK(_nick, args[2]));
            return;
        }
    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("KICK")));
    }
}

void Client::handleCommandInvite(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    (void) args;
    (void) server;
    if (args.size() > 2) {
        std::cout << "INVITE: checking if channel `" << channelNameFromArg(args[2]) << "` exists: " << server.channelExists(channelNameFromArg(args[2])) << std::endl;
        if (!server.channelExists(channelNameFromArg(args[2]))) {
            reply(ERR_NOSUCHCHANNEL(args[2]));
            return;
        }
        Channel &chan = server.getChannelByName(channelNameFromArg(args[2]));
        if (!chan.hasFd(_fd)) {
            reply(ERR_USERNOTINCHANNEL(_nick, chan.getName()));
            return;
        }
        if (server.nickExists(args[1])) {
            Client &target = server.getClientByNick(args[1]);
            chan.inviteFd(target.getFd());
            reply(RPL_INVITE(_nick, target.getNick(), target.getUser(), target.getHost(), chan.getName()));
            target.reply(INVITE(_nick, _user, _hostname, target.getNick(), chan.getName()));
        } else {
            reply(ERR_NOSUCHNICK(_nick, args[1]));
            return;
        }
    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("INVITE")));
    }
}

void Client::handleCommandPart(std::string &command, std::vector<std::string> &args, Server &server)  {
    if (args.size() > 1) {
        if (!server.channelExists(channelNameFromArg(args[1]))) {
            reply(ERR_NOSUCHCHANNEL(args[1]));
            return;
        }
        Channel &chan = server.getChannelByName(channelNameFromArg(args[1]));
        if (!chan.hasFd(_fd)) {
            reply(ERR_USERNOTINCHANNEL(_nick, chan.getName()));
            return;
        }
        chan.sendToAll(RPL_PART(_nick, _user, _hostname, chan.getName(), extractMessage(command)), server);
        chan.removeFd(_fd);
        if (chan.getClientFds().empty()) {
            server.removeChannel(chan.getName());
        }
    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("PART")));
    }
}

void Client::handleCommandQuit(std::string &command, std::vector<std::string> &args, Server &server)  {
    (void) command;
    (void) args;
    std::vector<Channel> &channels = server.getChannels();
    std::vector<std::string> chanNamesWithFd;
    for (size_t i = 0; i < channels.size(); ++i) {
        if (channels[i].hasFd(_fd)) {
            chanNamesWithFd.push_back(channels[i].getName());
        }
    }
    std::string msg = extractMessage(command);
    for (size_t i = 0; i < chanNamesWithFd.size(); ++i) {
        if (!server.channelExists(chanNamesWithFd[i])) {
            continue;
        }
        Channel &chan = server.getChannelByName(chanNamesWithFd[i]);
        chan.sendToAll(RPL_QUIT(_nick, _user, _hostname, channels[i].getName(), msg), server);
        chan.removeFd(_fd);
        if (chan.getClientFds().empty()) {
            server.removeChannel(chan.getName());
        }
    }
    reply(ERROR(msg));
    server.removeClient(_fd);
}

void Client::handleCommandMsg(std::string &command, std::vector<std::string> &args, Server &server)  {
    if (args.size() > 2) {
        if (!server.nickExists(args[1])) {
            reply(ERR_NOSUCHNICK(_nick, args[1]));
            return;
        }
        std::string message = extractMessage(command);
        Client &c = server.getClientByNick(args[1]);
        c.reply(PRIV_MSG_DM(_nick, _user, _hostname, c.getNick(), message));
    } else {
        reply(ERR_NEEDMOREPARAMS(std::string("PRIVMSG")));
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

        if (args[0] == "DEBUG:FDS") {
            server.debugPrintFds();
            continue;
        }
        if (args[0] == "DEBUG:CHANNELS") {
            server.debugPrintChannels();
            continue;
        }

        if (!_authenticated && args[0] != "PASS" && args[0] != "USER" && args[0] != "NICK") {
            std::cout << "Not registered, skipping command" << std::endl;
            reply(ERR_NOTREGISTERED);
            continue;
        }

        if (!handleCommand(*it, args, server)) {
            reply(ERR_UNKNOWNCOMMAND(args[0]));
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
        return;
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

void Client::setHost(const std::string &host) {
    _hostname = host;
}

std::string Client::getHost() {
    return _hostname;
}
