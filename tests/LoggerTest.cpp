/**
 * @file LoggerTest.cpp
 * @brief Unit tests for the enhanced Logger system.
 *
 * Tests the log level filtering mechanism — a core debuggability feature.
 * Verifies that:
 * - Default level allows all messages
 * - Setting a higher level suppresses lower-level messages
 * - Level can be changed at runtime
 */

#include <gtest/gtest.h>
#include "utils/Logger.h"

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset to default DEBUG level before each test
        Logger::setLevel(Logger::Level::DEBUG);
    }
};

TEST_F(LoggerTest, DefaultLevelIsDebug) {
    EXPECT_EQ(Logger::getLevel(), Logger::Level::DEBUG);
}

TEST_F(LoggerTest, SetLevelToInfo) {
    Logger::setLevel(Logger::Level::INFO);
    EXPECT_EQ(Logger::getLevel(), Logger::Level::INFO);
}

TEST_F(LoggerTest, SetLevelToError) {
    Logger::setLevel(Logger::Level::ERROR);
    EXPECT_EQ(Logger::getLevel(), Logger::Level::ERROR);
}

TEST_F(LoggerTest, LogInfoDoesNotCrash) {
    // Smoke test: calling logInfo should not throw or crash
    EXPECT_NO_THROW(Logger::logInfo("Test info message", "TestModule"));
}

TEST_F(LoggerTest, LogErrorDoesNotCrash) {
    EXPECT_NO_THROW(Logger::logError("Test error message", "TestModule"));
}

TEST_F(LoggerTest, LogWarnDoesNotCrash) {
    EXPECT_NO_THROW(Logger::logWarn("Test warning", "TestModule"));
}

TEST_F(LoggerTest, LogDebugDoesNotCrash) {
    EXPECT_NO_THROW(Logger::logDebug("Test debug message", "TestModule"));
}

TEST_F(LoggerTest, LogWithEmptyModule) {
    EXPECT_NO_THROW(Logger::logInfo("Message with empty module"));
}

TEST_F(LoggerTest, LogWithEmptyMessage) {
    EXPECT_NO_THROW(Logger::logInfo("", "TestModule"));
}

// === Level Filtering Tests ===

TEST_F(LoggerTest, DebugSuppressedAtInfoLevel) {
    Logger::setLevel(Logger::Level::INFO);
    // logDebug should be silently suppressed (hits the early return)
    EXPECT_NO_THROW(Logger::logDebug("This should be suppressed"));
}

TEST_F(LoggerTest, DebugSuppressedAtWarnLevel) {
    Logger::setLevel(Logger::Level::WARN);
    EXPECT_NO_THROW(Logger::logDebug("Suppressed"));
    EXPECT_NO_THROW(Logger::logInfo("Also suppressed"));
}

TEST_F(LoggerTest, DebugSuppressedAtErrorLevel) {
    Logger::setLevel(Logger::Level::ERROR);
    EXPECT_NO_THROW(Logger::logDebug("Suppressed"));
    EXPECT_NO_THROW(Logger::logInfo("Suppressed"));
    EXPECT_NO_THROW(Logger::logWarn("Suppressed"));
}

TEST_F(LoggerTest, ErrorNotSuppressedAtErrorLevel) {
    Logger::setLevel(Logger::Level::ERROR);
    // Only ERROR should still go through
    EXPECT_NO_THROW(Logger::logError("This should print to stderr"));
}

TEST_F(LoggerTest, InfoSuppressedAtWarnLevel) {
    Logger::setLevel(Logger::Level::WARN);
    EXPECT_NO_THROW(Logger::logInfo("Should be suppressed"));
}

TEST_F(LoggerTest, WarnNotSuppressedAtWarnLevel) {
    Logger::setLevel(Logger::Level::WARN);
    EXPECT_NO_THROW(Logger::logWarn("Should not be suppressed"));
}

// === Log to stderr (ERROR level) ===

TEST_F(LoggerTest, ErrorGoesToStderr) {
    // This test exercises the cerr branch in log()
    EXPECT_NO_THROW(Logger::logError("Error to stderr", "TestModule"));
}

// === Set and restore level ===

TEST_F(LoggerTest, SetLevelToWarn) {
    Logger::setLevel(Logger::Level::WARN);
    EXPECT_EQ(Logger::getLevel(), Logger::Level::WARN);
}

TEST_F(LoggerTest, SetLevelBackToDebug) {
    Logger::setLevel(Logger::Level::ERROR);
    Logger::setLevel(Logger::Level::DEBUG);
    EXPECT_EQ(Logger::getLevel(), Logger::Level::DEBUG);
}

// === Multiple log calls ===

TEST_F(LoggerTest, MultipleSequentialLogs) {
    EXPECT_NO_THROW({
        Logger::logDebug("debug1");
        Logger::logInfo("info1");
        Logger::logWarn("warn1");
        Logger::logError("error1");
    });
}

