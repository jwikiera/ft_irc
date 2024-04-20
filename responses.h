#ifndef FT_IRC_RESPONSES_H
#define FT_IRC_RESPONSES_H

#define ERR_NOTREGISTERED                       "451 :You have not registered"
#define ERR_ALREADYREGISTERED                   "462 :You may not register"
//#define ERR_PASSWDMISMATCH                      "464 :Password is incorrect"
#define ERR_PASSWDMISMATCH                      "ERROR :Access denied: Bad password?"
#define ERR_NONICKNAMEGIVEN                     "431 :Nickname not given"
#define ERR_NICKNAMEINUSE                       "433 :Nickname is already in use"

#define ERR_UNKNOWNCOMMAND(command)             "421 " + command + " :Unknown command"
#define ERR_NEEDMOREPARAMS(command)             "461 " + command + " :Not enough parameters"

#define ERR_TOOMANYCHANNELS(channel)            "405 " + channel + " :You have joined too many channels"
#define ERR_NOTONCHANNEL(channel)               "442 " + channel + " :You're not in that channel"
#define ERR_NOSUCHCHANNEL(channel)              "403 " + channel + " :No such channel"
#define ERR_BADCHANNELKEY(channel)              "475 " + channel + " :Cannot join channel (+k)"
#define ERR_CHANNELISFULL(channel)              "471 " + channel + " :Cannot join channel (+l)"
#define ERR_CANNOTSENDTOCHAN(channel)           "404 " + channel + " :Cannot send to channel"
#define ERR_CHANOPRIVSNEEDED(channel)           "482 " + channel + " :You're not channel operator"

#define ERR_NOSUCHNICK(nickname)                "401 " + nickname + " :No such nick/channel"
#define ERR_USERNOTINCHANNEL(nickname, channel) "441 " + nickname + " " + channel + " :They aren't on that channel"


#define RPL_CAP                                 "CAP * LS :multi-prefix"
#define RPL_CAP_REQ                             "CAP * ACK :multi-prefix"

#define RPL_WELCOME(source)                     "001 :Welcome to ft_irc"
#define RPL_NAMREPLY(source, channel, users)    "353 = " + channel + " :" + users
#define RPL_ENDOFNAMES(source, channel)         "366 " + channel + " :End of /NAMES list."

#define RPL_MOTD_MISSING(user)                  "422 " + user + " :MOTD file is missing"

/* Command Responses */

#define RPL_JOIN(channel)                       ":JOIN :" + channel
#define RPL_PART(channel)                       ":PART :" + channel
#define RPL_PING(token)                         ":PONG :" + token
#define RPL_PRIVMSG(target, message)            ":PRIVMSG " + target + " :" + message
#define RPL_NOTICE(target, message)             ":NOTICE " + target + " :" + message
#define RPL_QUIT(message)                       ":QUIT :Quit: " + message
#define RPL_KICK(channel, target, reason)       ":KICK " + channel + " " + target + " :" + reason
#define RPL_MODE(channel, modes, args)          ":MODE " + channel + " " + modes + " " + args

#endif //FT_IRC_RESPONSES_H
