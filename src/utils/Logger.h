#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <mutex>

/**
 * @class Logger
 * @brief Structured logging system with timestamps, log levels, and module tags.
 *
 * Provides thread-safe, structured logging to support debuggability.
 * Each log entry includes: [LEVEL] [TIMESTAMP] [MODULE] message
 *
 * Improvement over original: The original Logger only printed bare messages
 * with no timestamp or module context, making it impossible to trace
 * the execution flow or correlate log entries during debugging.
 */
class Logger {
public:
    enum class Level { DEBUG, INFO, WARN, ERROR };

    /**
     * @brief Log an informational message.
     * @param msg The message content.
     * @param module The originating module name (default: "GENERAL").
     */
    static void logInfo(const std::string& msg, const std::string& module = "GENERAL");

    /**
     * @brief Log an error message.
     * @param msg The message content.
     * @param module The originating module name (default: "GENERAL").
     */
    static void logError(const std::string& msg, const std::string& module = "GENERAL");

    /**
     * @brief Log a warning message.
     * @param msg The message content.
     * @param module The originating module name (default: "GENERAL").
     */
    static void logWarn(const std::string& msg, const std::string& module = "GENERAL");

    /**
     * @brief Log a debug message.
     * @param msg The message content.
     * @param module The originating module name (default: "GENERAL").
     */
    static void logDebug(const std::string& msg, const std::string& module = "GENERAL");

    /**
     * @brief Set the minimum log level. Messages below this level are suppressed.
     * @param level The minimum Level to output.
     */
    static void setLevel(Level level);

    /**
     * @brief Get the current minimum log level.
     */
    static Level getLevel();

private:
    static Level currentLevel;
    static std::mutex logMutex;

    static std::string getTimestamp();
    static std::string levelToString(Level level);
    static void log(Level level, const std::string& msg, const std::string& module);
};

#endif // LOGGER_H
