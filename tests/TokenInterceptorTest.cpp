/**
 * @file TokenInterceptorTest.cpp
 * @brief Unit tests for TokenInterceptor AOP logic.
 *
 * Tests the pre-handle authentication cut-point that guards all
 * protected endpoints. Covers:
 * - Valid token passes
 * - Empty token rejection
 * - Short token rejection
 * - Missing "Bearer" prefix rejection
 */

#include <gtest/gtest.h>
#include "interceptor/TokenInterceptor.h"
#include "utils/Exceptions.h"
#include "utils/Constants.h"

class TokenInterceptorTest : public ::testing::Test {
protected:
    TokenInterceptor interceptor;
};

TEST_F(TokenInterceptorTest, ValidToken_Passes) {
    EXPECT_TRUE(interceptor.preHandle("Bearer admin_secret_token"));
}

TEST_F(TokenInterceptorTest, EmptyToken_ThrowsUnauthorized) {
    EXPECT_THROW(interceptor.preHandle(""), UnauthorizedException);
}

TEST_F(TokenInterceptorTest, ShortToken_ThrowsUnauthorized) {
    EXPECT_THROW(interceptor.preHandle("short"), UnauthorizedException);
}

TEST_F(TokenInterceptorTest, MissingBearerPrefix_ThrowsUnauthorized) {
    EXPECT_THROW(interceptor.preHandle("Token admin_secret_token"), UnauthorizedException);
}

TEST_F(TokenInterceptorTest, BearerOnly_ThrowsUnauthorized) {
    // "Bearer" alone is only 6 chars, which is < MIN_TOKEN_LEN(10)
    EXPECT_THROW(interceptor.preHandle("Bearer"), UnauthorizedException);
}

TEST_F(TokenInterceptorTest, ValidLongToken_Passes) {
    EXPECT_TRUE(interceptor.preHandle("Bearer some_very_long_valid_token_string_here"));
}
