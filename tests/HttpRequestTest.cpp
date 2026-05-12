#include <gtest/gtest.h>
#include "network/HttpRequest.h"

// === Constructor Tests ===

TEST(HttpRequestTest, Constructor_SetsMethodAndPath) {
    HttpRequest req("GET", "/api/items");
    EXPECT_EQ(req.getMethod(), "GET");
    EXPECT_EQ(req.getPath(), "/api/items");
}

// === Header Tests ===

TEST(HttpRequestTest, SetAndGetHeader) {
    HttpRequest req("GET", "/");
    req.setHeader("Authorization", "Bearer token123");
    EXPECT_EQ(req.getHeader("Authorization"), "Bearer token123");
}

TEST(HttpRequestTest, GetHeader_Missing_ReturnsEmpty) {
    HttpRequest req("GET", "/");
    EXPECT_EQ(req.getHeader("NonExistent"), "");
}

TEST(HttpRequestTest, SetHeader_Overwrite) {
    HttpRequest req("GET", "/");
    req.setHeader("Key", "value1");
    req.setHeader("Key", "value2");
    EXPECT_EQ(req.getHeader("Key"), "value2");
}

// === Query Param Tests ===

TEST(HttpRequestTest, SetAndGetQueryParam) {
    HttpRequest req("GET", "/api/items");
    req.setQueryParam("id", "42");
    EXPECT_EQ(req.getQueryParam("id"), "42");
}

TEST(HttpRequestTest, GetQueryParam_Missing_ReturnsEmpty) {
    HttpRequest req("GET", "/");
    EXPECT_EQ(req.getQueryParam("missing"), "");
}

// === Body Tests ===

TEST(HttpRequestTest, SetAndGetBody) {
    HttpRequest req("POST", "/api/items");
    req.setBody("{\"name\":\"Sword\"}");
    EXPECT_EQ(req.getBody(), "{\"name\":\"Sword\"}");
}

TEST(HttpRequestTest, GetBody_Default_Empty) {
    HttpRequest req("GET", "/");
    EXPECT_EQ(req.getBody(), "");
}

// === parseRawRequest Tests ===

TEST(HttpRequestTest, ParseRawRequest_ValidRequest) {
    HttpRequest req("", "");
    req.parseRawRequest("GET /api/items HTTP/1.1");
    EXPECT_EQ(req.getMethod(), "GET");
    EXPECT_EQ(req.getPath(), "/api/items");
}

TEST(HttpRequestTest, ParseRawRequest_PostRequest) {
    HttpRequest req("", "");
    req.parseRawRequest("POST /api/auth/login HTTP/1.1");
    EXPECT_EQ(req.getMethod(), "POST");
    EXPECT_EQ(req.getPath(), "/api/auth/login");
}

TEST(HttpRequestTest, ParseRawRequest_EmptyString) {
    HttpRequest req("GET", "/original");
    req.parseRawRequest("");
    // Should not crash, method/path unchanged
    EXPECT_EQ(req.getMethod(), "GET");
    EXPECT_EQ(req.getPath(), "/original");
}

TEST(HttpRequestTest, ParseRawRequest_NoSpaces) {
    HttpRequest req("GET", "/original");
    req.parseRawRequest("NOSPACES");
    // No space found, method/path should remain original
    EXPECT_EQ(req.getMethod(), "GET");
}
