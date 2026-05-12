#include <gtest/gtest.h>
#include "controller/Dispatcher.h"
#include "controller/ItemController.h"
#include "controller/AuthController.h"
#include "repository/MockDatabase.h"

class DispatcherTest : public ::testing::Test {
protected:
    ItemController itemController;
    AuthController authController;
    Dispatcher* dispatcher;

    void SetUp() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();

        // Add a test user for login
        User admin;
        admin.userId = 1;
        admin.username = "admin";
        admin.password = "123456";
        admin.token = "Bearer admin_secret_token";
        MockDatabase::users.push_back(admin);

        dispatcher = new Dispatcher(&itemController, &authController);
    }

    void TearDown() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
        delete dispatcher;
    }
};

// === Login Route ===

TEST_F(DispatcherTest, Login_Success) {
    HttpRequest req("POST", "/api/auth/login");
    req.setQueryParam("username", "admin");
    req.setQueryParam("password", "123456");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 200);
    EXPECT_NE(resp.getBody().find("token"), std::string::npos);
}

TEST_F(DispatcherTest, Login_Failure_WrongPassword) {
    HttpRequest req("POST", "/api/auth/login");
    req.setQueryParam("username", "admin");
    req.setQueryParam("password", "wrong");
    HttpResponse resp = dispatcher->dispatch(req);
    // AuthController catches exception and returns empty token
    EXPECT_EQ(resp.getStatusCode(), 401);
}

// === Protected Route Without Token ===

TEST_F(DispatcherTest, ProtectedRoute_NoToken_Returns401) {
    HttpRequest req("GET", "/api/items");
    req.setQueryParam("id", "1");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 401);
}

// === Item CRUD Routes ===

TEST_F(DispatcherTest, CreateItem_WithToken) {
    HttpRequest req("POST", "/api/items");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    req.setQueryParam("id", "1");
    req.setQueryParam("name", "Sword");
    req.setQueryParam("sellIn", "10");
    req.setQueryParam("quality", "20");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 201);
    EXPECT_NE(resp.getBody().find("created"), std::string::npos);
}

TEST_F(DispatcherTest, GetItem_WithToken_MissingId) {
    HttpRequest req("GET", "/api/items");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    // No id param
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 400);
    EXPECT_NE(resp.getBody().find("Missing id"), std::string::npos);
}

TEST_F(DispatcherTest, GetItem_WithToken_ValidId) {
    MockDatabase::items.push_back(Item(1, "Sword", 10, 20));
    HttpRequest req("GET", "/api/items");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    req.setQueryParam("id", "1");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 200);
}

TEST_F(DispatcherTest, UpdateItem_WithToken) {
    MockDatabase::items.push_back(Item(1, "Sword", 10, 20));
    HttpRequest req("PUT", "/api/items");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    req.setQueryParam("id", "1");
    req.setQueryParam("name", "Golden Sword");
    req.setQueryParam("sellIn", "15");
    req.setQueryParam("quality", "30");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 200);
    EXPECT_NE(resp.getBody().find("updated"), std::string::npos);
}

TEST_F(DispatcherTest, DeleteItem_WithToken) {
    MockDatabase::items.push_back(Item(1, "Sword", 10, 20));
    HttpRequest req("DELETE", "/api/items");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    req.setQueryParam("id", "1");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 200);
    EXPECT_NE(resp.getBody().find("deleted"), std::string::npos);
}

// === Admin Routes ===

TEST_F(DispatcherTest, DailySettlement_WithToken) {
    MockDatabase::items.push_back(Item(1, "Sword", 10, 20));
    HttpRequest req("POST", "/api/admin/settle");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 200);
    EXPECT_NE(resp.getBody().find("settled"), std::string::npos);
}

TEST_F(DispatcherTest, InventoryWarnings_WithToken) {
    HttpRequest req("GET", "/api/admin/warnings");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 200);
}

// === 404 Route ===

TEST_F(DispatcherTest, UnknownRoute_Returns404) {
    HttpRequest req("GET", "/api/unknown");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 404);
    EXPECT_NE(resp.getBody().find("Not found"), std::string::npos);
}

// === Invalid Parameter ===

TEST_F(DispatcherTest, InvalidParamFormat_Returns400) {
    HttpRequest req("GET", "/api/items");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    req.setQueryParam("id", "not_a_number");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 400);
}

// === Search Route ===

TEST_F(DispatcherTest, SearchRoute_WithToken) {
    MockDatabase::init();  // Initialize the SQLite DB
    HttpRequest req("GET", "/api/admin/search");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    req.setQueryParam("username", "admin");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 200);
}

TEST_F(DispatcherTest, SearchRoute_NonExistentUser) {
    MockDatabase::init();
    HttpRequest req("GET", "/api/admin/search");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    req.setQueryParam("username", "nobody");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 200);
}

// === Exception Handling ===

TEST_F(DispatcherTest, GetItem_NotFound_Returns500) {
    HttpRequest req("GET", "/api/items");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    req.setQueryParam("id", "999");
    HttpResponse resp = dispatcher->dispatch(req);
    // ItemNotFoundException is a std::exception subclass -> caught by generic handler
    EXPECT_EQ(resp.getStatusCode(), 500);
}

TEST_F(DispatcherTest, CreateItem_EmptyName_Returns500) {
    HttpRequest req("POST", "/api/items");
    req.setHeader("Authorization", "Bearer admin_secret_token");
    req.setQueryParam("id", "1");
    req.setQueryParam("name", "");
    req.setQueryParam("sellIn", "10");
    req.setQueryParam("quality", "20");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 500);
}

TEST_F(DispatcherTest, Login_EmptyCredentials) {
    HttpRequest req("POST", "/api/auth/login");
    req.setQueryParam("username", "");
    req.setQueryParam("password", "");
    HttpResponse resp = dispatcher->dispatch(req);
    EXPECT_EQ(resp.getStatusCode(), 401);
}
