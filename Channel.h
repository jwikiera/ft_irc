#ifndef FT_IRC_CHANNEL_H
#define FT_IRC_CHANNEL_H

#include <vector>

class Channel {
private:
    unsigned int mode;
    std::vector<int> _clientFds;
    std::vector<int> _opFds;
};


#endif //FT_IRC_CHANNEL_H
