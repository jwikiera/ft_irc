#include "Util.h"

/* Source: https://stackoverflow.com/a/36169979 */
 std::string Util::trimSpace(const std::string &str) {
    if (str.empty()) {
        return (str);
    }
    std::string::size_type i, j;
    i = 0;
    while (i<str.size() && isspace(str[i])) {
        i ++;
    }
    if (i == str.size()) {
        return std::string(); // empty string
    }
    j = str.size() - 1;
    while (isspace(str[j])) {
        j --;
    }
    return (str.substr(i, j - i + 1));
}

std::string Util::removeFirstChar(const std::string &str) {
    if (str.empty()) {
        return str;
    }
    const char *newStr = str.c_str() + 1;
    return std::string(newStr);
}
