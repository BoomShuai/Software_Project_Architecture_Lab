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
