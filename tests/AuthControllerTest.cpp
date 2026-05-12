#include <gtest/gtest.h>
#include "controller/AuthController.h"
#include "repository/MockDatabase.h"

class AuthControllerTest : public ::testing::Test {
protected:
    AuthController controller;
    void SetUp() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
        User admin;
        admin.userId = 1;
        admin.username = "admin";
        admin.password = "123456";
        admin.token = "Bearer admin_secret_token";
        MockDatabase::users.push_back(admin);
    }
    void TearDown() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
};

TEST_F(AuthControllerTest, Login_Success_ReturnsToken) {
    std::string token = controller.handleLoginRequest("admin", "123456");
    EXPECT_FALSE(token.empty());
    EXPECT_NE(token.find("Bearer"), std::string::npos);
}

TEST_F(AuthControllerTest, Login_WrongPassword_ReturnsEmpty) {
    std::string token = controller.handleLoginRequest("admin", "wrongpass");
    EXPECT_TRUE(token.empty());
}

TEST_F(AuthControllerTest, Login_UnknownUser_ReturnsEmpty) {
    std::string token = controller.handleLoginRequest("unknown", "123456");
    EXPECT_TRUE(token.empty());
}

TEST_F(AuthControllerTest, Login_EmptyCredentials_ReturnsEmpty) {
    std::string token = controller.handleLoginRequest("", "");
    EXPECT_TRUE(token.empty());
}

TEST_F(AuthControllerTest, Login_EmptyUsername_ReturnsEmpty) {
    std::string token = controller.handleLoginRequest("", "123456");
    EXPECT_TRUE(token.empty());
}

TEST_F(AuthControllerTest, Login_EmptyPassword_ReturnsEmpty) {
    std::string token = controller.handleLoginRequest("admin", "");
    EXPECT_TRUE(token.empty());
}
