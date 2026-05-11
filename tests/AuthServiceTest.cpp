/**
 * @file AuthServiceTest.cpp
 * @brief Unit tests for AuthService authentication logic.
 *
 * Tests cover:
 * - Successful login with correct credentials
 * - Failed login with wrong credentials
 * - Empty input boundary cases
 * - Token format verification
 */

#include <gtest/gtest.h>
#include "service/AuthService.h"
#include "repository/MockDatabase.h"
#include "utils/Exceptions.h"

class AuthServiceTest : public ::testing::Test {
protected:
    AuthService service;

    void SetUp() override {
        MockDatabase::users.clear();

        User admin;
        admin.userId = 1;
        admin.username = "admin";
        admin.password = "123456";
        admin.token = "Bearer admin_secret_token";
        MockDatabase::users.push_back(admin);
    }

    void TearDown() override {
        MockDatabase::users.clear();
    }
};

TEST_F(AuthServiceTest, Login_HappyPath) {
    std::string token = service.login("admin", "123456");
    EXPECT_EQ(token, "Bearer admin_secret_token");
}

TEST_F(AuthServiceTest, Login_ThrowsOnWrongPassword) {
    EXPECT_THROW(service.login("admin", "wrong_password"), AuthenticationException);
}

TEST_F(AuthServiceTest, Login_ThrowsOnWrongUsername) {
    EXPECT_THROW(service.login("unknown_user", "123456"), AuthenticationException);
}

TEST_F(AuthServiceTest, Login_ThrowsOnEmptyUsername) {
    EXPECT_THROW(service.login("", "123456"), ValidationException);
}

TEST_F(AuthServiceTest, Login_ThrowsOnEmptyPassword) {
    EXPECT_THROW(service.login("admin", ""), ValidationException);
}

TEST_F(AuthServiceTest, Login_ThrowsOnBothEmpty) {
    EXPECT_THROW(service.login("", ""), ValidationException);
}

TEST_F(AuthServiceTest, Login_TokenContainsBearerPrefix) {
    std::string token = service.login("admin", "123456");
    EXPECT_TRUE(token.find("Bearer") == 0);
}
