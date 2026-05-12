#include <gtest/gtest.h>
#include "network/HttpResponse.h"

TEST(HttpResponseTest, DefaultStatusCode_Is200) {
    HttpResponse resp;
    EXPECT_EQ(resp.getStatusCode(), 200);
}

TEST(HttpResponseTest, SetStatusCode) {
    HttpResponse resp;
    resp.setStatusCode(404);
    EXPECT_EQ(resp.getStatusCode(), 404);
}

TEST(HttpResponseTest, SetAndGetBody) {
    HttpResponse resp;
    resp.setBody("{\"error\":\"not found\"}");
    EXPECT_EQ(resp.getBody(), "{\"error\":\"not found\"}");
}

TEST(HttpResponseTest, SetHeader) {
    HttpResponse resp;
    resp.setHeader("X-Custom", "value");
    // Verify via raw response
    std::string raw = resp.generateRawResponse();
    EXPECT_NE(raw.find("X-Custom: value"), std::string::npos);
}

TEST(HttpResponseTest, GenerateRawResponse_ContainsStatusLine) {
    HttpResponse resp;
    resp.setStatusCode(200);
    std::string raw = resp.generateRawResponse();
    EXPECT_NE(raw.find("HTTP/1.1 200 OK"), std::string::npos);
}

TEST(HttpResponseTest, GenerateRawResponse_ContainsContentType) {
    HttpResponse resp;
    std::string raw = resp.generateRawResponse();
    EXPECT_NE(raw.find("Content-Type: application/json"), std::string::npos);
}

TEST(HttpResponseTest, GenerateRawResponse_ContainsBody) {
    HttpResponse resp;
    resp.setBody("hello world");
    std::string raw = resp.generateRawResponse();
    EXPECT_NE(raw.find("hello world"), std::string::npos);
}

TEST(HttpResponseTest, GenerateRawResponse_404) {
    HttpResponse resp;
    resp.setStatusCode(404);
    std::string raw = resp.generateRawResponse();
    EXPECT_NE(raw.find("404"), std::string::npos);
}

TEST(HttpResponseTest, GenerateRawResponse_500) {
    HttpResponse resp;
    resp.setStatusCode(500);
    resp.setBody("{\"error\":\"internal\"}");
    std::string raw = resp.generateRawResponse();
    EXPECT_NE(raw.find("500"), std::string::npos);
    EXPECT_NE(raw.find("{\"error\":\"internal\"}"), std::string::npos);
}
