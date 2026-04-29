#include "Logger.h"
#include <iostream>

void Logger::logInfo(std::string msg) {
    std::cout << "[INFO] " << msg << std::endl;
}

void Logger::logError(std::string msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
}
