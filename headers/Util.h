#ifndef FT_IRC_UTIL_H
#define FT_IRC_UTIL_H

#include <string>

class Util {
public:
    static std::string trimSpace(const std::string &str);
    static std::string removeFirstChar(const std::string &str);
};


#endif //FT_IRC_UTIL_H
