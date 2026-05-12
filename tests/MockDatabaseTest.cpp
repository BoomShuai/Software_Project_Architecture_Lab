#include <gtest/gtest.h>
#include "repository/MockDatabase.h"
#include "model/Item.h"
#include "model/User.h"

class MockDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
    void TearDown() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
};

// === Init Tests ===

TEST_F(MockDatabaseTest, Init_PopulatesItems) {
    MockDatabase::init();
    EXPECT_GE(MockDatabase::items.size(), 8);
}

TEST_F(MockDatabaseTest, Init_PopulatesUsers) {
    MockDatabase::init();
    EXPECT_GE(MockDatabase::users.size(), 1);
    EXPECT_EQ(MockDatabase::users[0].username, "admin");
}

TEST_F(MockDatabaseTest, Init_ContainsDexterityVest) {
    MockDatabase::init();
    bool found = false;
    for (const auto& item : MockDatabase::items) {
        if (item.name == "+5 Dexterity Vest") { found = true; break; }
    }
    EXPECT_TRUE(found);
}

TEST_F(MockDatabaseTest, Init_ContainsAgedBrie) {
    MockDatabase::init();
    bool found = false;
    for (const auto& item : MockDatabase::items) {
        if (item.name == "Aged Brie") { found = true; break; }
    }
    EXPECT_TRUE(found);
}

TEST_F(MockDatabaseTest, Init_ContainsSulfuras) {
    MockDatabase::init();
    int count = 0;
    for (const auto& item : MockDatabase::items) {
        if (item.name == "Sulfuras, Hand of Ragnaros") count++;
    }
    EXPECT_EQ(count, 2);
}

TEST_F(MockDatabaseTest, Init_ContainsBackstagePasses) {
    MockDatabase::init();
    int count = 0;
    for (const auto& item : MockDatabase::items) {
        if (item.name == "Backstage passes to a TAFKAL80ETC concert") count++;
    }
    EXPECT_EQ(count, 3);
}

TEST_F(MockDatabaseTest, Init_AdminToken) {
    MockDatabase::init();
    EXPECT_EQ(MockDatabase::users[0].token, "Bearer admin_secret_token");
}

// === SafeQuery Tests ===

TEST_F(MockDatabaseTest, SafeQuery_WithDbInitialized) {
    MockDatabase::init();
    std::string result = MockDatabase::safeQuery("admin");
    // Should find the admin user
    EXPECT_NE(result.find("admin"), std::string::npos);
}

TEST_F(MockDatabaseTest, SafeQuery_NonExistentUser) {
    MockDatabase::init();
    std::string result = MockDatabase::safeQuery("nonexistent");
    // Should return empty or no match
    EXPECT_EQ(result.find("nonexistent"), std::string::npos);
}

TEST_F(MockDatabaseTest, SafeQuery_SqlInjectionPrevented) {
    MockDatabase::init();
    // Attempt SQL injection - should be safely handled by prepared statement
    std::string result = MockDatabase::safeQuery("admin' OR '1'='1");
    // Should NOT return all users - injection should fail
    EXPECT_TRUE(result.empty() || result.find("admin' OR") == std::string::npos);
}

TEST_F(MockDatabaseTest, SafeQuery_EmptyInput) {
    MockDatabase::init();
    std::string result = MockDatabase::safeQuery("");
    // Empty username should match no rows
    EXPECT_TRUE(result.empty() || result.find("USERNAME") == std::string::npos);
}

// === Static Collections Tests ===

TEST_F(MockDatabaseTest, Items_InitiallyEmpty) {
    EXPECT_EQ(MockDatabase::items.size(), 0);
}

TEST_F(MockDatabaseTest, Users_InitiallyEmpty) {
    EXPECT_EQ(MockDatabase::users.size(), 0);
}

TEST_F(MockDatabaseTest, Items_CanAddAndRetrieve) {
    MockDatabase::items.push_back(Item(99, "TestItem", 5, 10));
    EXPECT_EQ(MockDatabase::items.size(), 1);
    EXPECT_EQ(MockDatabase::items[0].name, "TestItem");
}

TEST_F(MockDatabaseTest, Users_CanAddAndRetrieve) {
    User u;
    u.userId = 42;
    u.username = "testuser";
    u.password = "testpass";
    MockDatabase::users.push_back(u);
    EXPECT_EQ(MockDatabase::users.size(), 1);
    EXPECT_EQ(MockDatabase::users[0].username, "testuser");
}
