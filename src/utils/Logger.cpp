#include "Logger.h"

Logger::Level Logger::currentLevel = Logger::Level::DEBUG;
std::mutex Logger::logMutex;

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::DEBUG: return "DEBUG";
        case Level::INFO:  return "INFO ";
        case Level::WARN:  return "WARN ";
        case Level::ERROR: return "ERROR";
        default:           return "UNKNOWN";
    }
}

void Logger::log(Level level, const std::string& msg, const std::string& module) {
    if (level < currentLevel) return;

    std::lock_guard<std::mutex> guard(logMutex);
    std::ostream& out = (level >= Level::ERROR) ? std::cerr : std::cout;
    out << "[" << levelToString(level) << "] "
        << "[" << getTimestamp() << "] "
        << "[" << module << "] "
        << msg << std::endl;
}

void Logger::logInfo(const std::string& msg, const std::string& module) {
    log(Level::INFO, msg, module);
}

void Logger::logError(const std::string& msg, const std::string& module) {
    log(Level::ERROR, msg, module);
}

void Logger::logWarn(const std::string& msg, const std::string& module) {
    log(Level::WARN, msg, module);
}

void Logger::logDebug(const std::string& msg, const std::string& module) {
    log(Level::DEBUG, msg, module);
}

void Logger::setLevel(Level level) {
    currentLevel = level;
}

Logger::Level Logger::getLevel() {
    return currentLevel;
}
