#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <string>
#include <vector>

class StringUtil {
public:
    static std::string trim(const std::string& str);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string toUpper(const std::string& str);
    static std::string toLower(const std::string& str);
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
};

#endif // STRING_UTIL_H
