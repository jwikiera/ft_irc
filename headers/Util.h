#ifndef FT_IRC_UTIL_H
#define FT_IRC_UTIL_H

#include <string>
#include <sstream>

class Util {
public:
    static std::string trimSpace(const std::string &str);
    static std::string removeFirstChar(const std::string &str);
    static std::string streamItoa(int n);
};


#endif //FT_IRC_UTIL_H
