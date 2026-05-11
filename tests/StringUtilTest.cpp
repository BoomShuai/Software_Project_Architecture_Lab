/**
 * @file StringUtilTest.cpp
 * @brief Unit tests for StringUtil utility functions.
 *
 * Tests all string manipulation helpers:
 * - trim, split, toUpper, toLower, startsWith, endsWith
 * - Empty string edge cases
 */

#include <gtest/gtest.h>
#include "utils/StringUtil.h"

// === TRIM ===
TEST(StringUtilTest, Trim_RemovesLeadingAndTrailingSpaces) {
    EXPECT_EQ(StringUtil::trim("  hello  "), "hello");
}

TEST(StringUtilTest, Trim_RemovesTabs) {
    EXPECT_EQ(StringUtil::trim("\thello\t"), "hello");
}

TEST(StringUtilTest, Trim_EmptyString) {
    EXPECT_EQ(StringUtil::trim(""), "");
}

TEST(StringUtilTest, Trim_AllWhitespace) {
    EXPECT_EQ(StringUtil::trim("   \t\n  "), "");
}

TEST(StringUtilTest, Trim_NoWhitespace) {
    EXPECT_EQ(StringUtil::trim("hello"), "hello");
}

// === SPLIT ===
TEST(StringUtilTest, Split_ByComma) {
    auto result = StringUtil::split("a,b,c", ',');
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");
}

TEST(StringUtilTest, Split_SingleElement) {
    auto result = StringUtil::split("hello", ',');
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello");
}

TEST(StringUtilTest, Split_EmptyString) {
    auto result = StringUtil::split("", ',');
    // std::getline on empty produces one empty token
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "");
}

// === TO UPPER ===
TEST(StringUtilTest, ToUpper_LowercaseInput) {
    EXPECT_EQ(StringUtil::toUpper("hello"), "HELLO");
}

TEST(StringUtilTest, ToUpper_MixedCase) {
    EXPECT_EQ(StringUtil::toUpper("Hello World"), "HELLO WORLD");
}

TEST(StringUtilTest, ToUpper_EmptyString) {
    EXPECT_EQ(StringUtil::toUpper(""), "");
}

// === TO LOWER ===
TEST(StringUtilTest, ToLower_UppercaseInput) {
    EXPECT_EQ(StringUtil::toLower("HELLO"), "hello");
}

TEST(StringUtilTest, ToLower_EmptyString) {
    EXPECT_EQ(StringUtil::toLower(""), "");
}

// === STARTS WITH ===
TEST(StringUtilTest, StartsWith_True) {
    EXPECT_TRUE(StringUtil::startsWith("Bearer token", "Bearer"));
}

TEST(StringUtilTest, StartsWith_False) {
    EXPECT_FALSE(StringUtil::startsWith("Token bearer", "Bearer"));
}

TEST(StringUtilTest, StartsWith_PrefixLongerThanStr) {
    EXPECT_FALSE(StringUtil::startsWith("hi", "hello world"));
}

TEST(StringUtilTest, StartsWith_EmptyPrefix) {
    EXPECT_TRUE(StringUtil::startsWith("hello", ""));
}

// === ENDS WITH ===
TEST(StringUtilTest, EndsWith_True) {
    EXPECT_TRUE(StringUtil::endsWith("report.csv", ".csv"));
}

TEST(StringUtilTest, EndsWith_False) {
    EXPECT_FALSE(StringUtil::endsWith("report.csv", ".xml"));
}

TEST(StringUtilTest, EndsWith_SuffixLongerThanStr) {
    EXPECT_FALSE(StringUtil::endsWith("hi", "hello world"));
}
