#ifndef LOGGER_H
#define LOGGER_H

#include <string>

class Logger {
public:
    static void logInfo(std::string msg);
    static void logError(std::string msg);
};

#endif // LOGGER_H
