#include <gtest/gtest.h>
#include "service/CacheService.h"
#include "network/HttpRequest.h"
#include "utils/StringUtil.h"

// === StringUtil::escapeJson ===

TEST(ScalabilityTest, EscapeJson_EscapesQuotesAndNewlines) {
    std::string in = "line1\n\"quoted\"\tend";
    std::string out = StringUtil::escapeJson(in);
    EXPECT_NE(out.find("\\n"), std::string::npos);
    EXPECT_NE(out.find("\\\""), std::string::npos);
    EXPECT_NE(out.find("\\t"), std::string::npos);
    EXPECT_EQ(out.find('\n'), std::string::npos); // raw newline gone
}

TEST(ScalabilityTest, EscapeJson_PlainStringUnchanged) {
    EXPECT_EQ(StringUtil::escapeJson("hello world"), "hello world");
}

// === HttpRequest full raw parsing (query string + headers + body) ===

TEST(ScalabilityTest, ParseRaw_ExtractsQueryParams) {
    HttpRequest req("", "");
    req.parseRawRequest(
        "GET /api/items?id=42&name=Sword HTTP/1.1\r\nHost: x\r\n\r\n");
    EXPECT_EQ(req.getPath(), "/api/items");
    EXPECT_EQ(req.getQueryParam("id"), "42");
    EXPECT_EQ(req.getQueryParam("name"), "Sword");
}

TEST(ScalabilityTest, ParseRaw_ExtractsHeaders) {
    HttpRequest req("", "");
    req.parseRawRequest(
        "POST /api/items HTTP/1.1\r\nAuthorization: Bearer tok\r\n\r\n");
    EXPECT_EQ(req.getHeader("Authorization"), "Bearer tok");
}

TEST(ScalabilityTest, ParseRaw_UrlDecodesParams) {
    HttpRequest req("", "");
    req.parseRawRequest("GET /s?q=a%20b%2Bc HTTP/1.1\r\n\r\n");
    EXPECT_EQ(req.getQueryParam("q"), "a b+c");
}

TEST(ScalabilityTest, ParseRaw_RequestLineOnly_StillWorks) {
    // Backward compatibility with the original single-line behaviour.
    HttpRequest req("", "");
    req.parseRawRequest("GET /api/items HTTP/1.1");
    EXPECT_EQ(req.getMethod(), "GET");
    EXPECT_EQ(req.getPath(), "/api/items");
}

// === CacheService (in-process LRU backend) ===

TEST(ScalabilityTest, Cache_SetThenGet_ReturnsValue) {
    CacheService cache;
    cache.set("k", "v");
    auto got = cache.get("k");
    ASSERT_TRUE(got.has_value());
    EXPECT_EQ(*got, "v");
}

TEST(ScalabilityTest, Cache_Miss_ReturnsNullopt) {
    CacheService cache;
    EXPECT_FALSE(cache.get("absent").has_value());
}

TEST(ScalabilityTest, Cache_Invalidate_RemovesEntry) {
    CacheService cache;
    cache.set("k", "v");
    cache.invalidate("k");
    EXPECT_FALSE(cache.get("k").has_value());
}

TEST(ScalabilityTest, Cache_DefaultBackend_IsInProcess) {
    CacheService cache;
    EXPECT_FALSE(cache.usingRedis());
    EXPECT_EQ(cache.backendName(), "In-process LRU");
}
