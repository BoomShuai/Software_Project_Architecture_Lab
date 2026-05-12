#include <gtest/gtest.h>
#include "utils/Exceptions.h"

// Test all exception types to ensure their constructors and what() messages work

TEST(ExceptionsTest, ItemNotFoundException_Message) {
    ItemNotFoundException ex(42);
    std::string msg = ex.what();
    EXPECT_NE(msg.find("42"), std::string::npos);
    EXPECT_NE(msg.find("not found"), std::string::npos);
}

TEST(ExceptionsTest, ItemNotFoundException_DifferentId) {
    ItemNotFoundException ex(999);
    std::string msg = ex.what();
    EXPECT_NE(msg.find("999"), std::string::npos);
}

TEST(ExceptionsTest, AuthenticationException_Message) {
    AuthenticationException ex("bad credentials");
    std::string msg = ex.what();
    EXPECT_NE(msg.find("bad credentials"), std::string::npos);
    EXPECT_NE(msg.find("Authentication failed"), std::string::npos);
}

TEST(ExceptionsTest, ValidationException_Message) {
    ValidationException ex("name is empty");
    std::string msg = ex.what();
    EXPECT_NE(msg.find("name is empty"), std::string::npos);
    EXPECT_NE(msg.find("Validation error"), std::string::npos);
}

TEST(ExceptionsTest, UnauthorizedException_Message) {
    UnauthorizedException ex;
    std::string msg = ex.what();
    EXPECT_NE(msg.find("Unauthorized"), std::string::npos);
}

TEST(ExceptionsTest, ItemNotFoundException_IsCatchableAsRuntimeError) {
    try {
        throw ItemNotFoundException(1);
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("1"), std::string::npos);
    }
}

TEST(ExceptionsTest, AuthenticationException_IsCatchableAsRuntimeError) {
    try {
        throw AuthenticationException("test");
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("test"), std::string::npos);
    }
}

TEST(ExceptionsTest, ValidationException_IsCatchableAsRuntimeError) {
    try {
        throw ValidationException("test");
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("test"), std::string::npos);
    }
}

TEST(ExceptionsTest, UnauthorizedException_IsCatchableAsRuntimeError) {
    try {
        throw UnauthorizedException();
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("Unauthorized"), std::string::npos);
    }
}

TEST(ExceptionsTest, AllExceptions_CatchableAsStdException) {
    EXPECT_THROW(throw ItemNotFoundException(0), std::exception);
    EXPECT_THROW(throw AuthenticationException("x"), std::exception);
    EXPECT_THROW(throw ValidationException("x"), std::exception);
    EXPECT_THROW(throw UnauthorizedException(), std::exception);
}
