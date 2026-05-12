#include <gtest/gtest.h>
#include "service/GildedRoseService.h"
#include "repository/MockDatabase.h"
#include "utils/Constants.h"

class GildedRoseServiceTest : public ::testing::Test {
protected:
    GildedRoseService service;
    void SetUp() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
    void TearDown() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
};

// === Daily Settlement Tests ===

TEST_F(GildedRoseServiceTest, Settlement_EmptyInventory) {
    EXPECT_NO_THROW(service.dailySettlement());
}

TEST_F(GildedRoseServiceTest, Settlement_NormalItem_DecreasesQualityAndSellIn) {
    MockDatabase::items.push_back(Item(1, "+5 Dexterity Vest", 10, 20));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].sellIn, 9);
    EXPECT_EQ(MockDatabase::items[0].quality, 19);
}

TEST_F(GildedRoseServiceTest, Settlement_AgedBrie_IncreasesQuality) {
    MockDatabase::items.push_back(Item(1, Constants::AGED_BRIE, 2, 0));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].sellIn, 1);
    EXPECT_EQ(MockDatabase::items[0].quality, 1);
}

TEST_F(GildedRoseServiceTest, Settlement_Sulfuras_NeverChanges) {
    MockDatabase::items.push_back(Item(1, Constants::SULFURAS, 0, 80));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].sellIn, 0);
    EXPECT_EQ(MockDatabase::items[0].quality, 80);
}

TEST_F(GildedRoseServiceTest, Settlement_BackstagePass_IncreasesNearConcert) {
    MockDatabase::items.push_back(Item(1, Constants::BACKSTAGE_PASS, 5, 20));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].quality, 23);
}

TEST_F(GildedRoseServiceTest, Settlement_BackstagePass_ZeroAfterConcert) {
    MockDatabase::items.push_back(Item(1, Constants::BACKSTAGE_PASS, 0, 50));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].quality, 0);
}

TEST_F(GildedRoseServiceTest, Settlement_MultipleItems) {
    MockDatabase::items.push_back(Item(1, "+5 Dexterity Vest", 10, 20));
    MockDatabase::items.push_back(Item(2, Constants::AGED_BRIE, 2, 0));
    MockDatabase::items.push_back(Item(3, Constants::SULFURAS, 0, 80));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items.size(), 3);
    EXPECT_EQ(MockDatabase::items[0].quality, 19);
    EXPECT_EQ(MockDatabase::items[1].quality, 1);
    EXPECT_EQ(MockDatabase::items[2].quality, 80);
}

TEST_F(GildedRoseServiceTest, Settlement_NormalItem_ExpiredDegradesTwice) {
    MockDatabase::items.push_back(Item(1, "Elixir", 0, 10));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].quality, 8);
}

TEST_F(GildedRoseServiceTest, Settlement_QualityNeverNegative) {
    MockDatabase::items.push_back(Item(1, "Elixir", 0, 0));
    service.dailySettlement();
    EXPECT_GE(MockDatabase::items[0].quality, 0);
}

// === Additional Branch Coverage ===

TEST_F(GildedRoseServiceTest, Settlement_BackstagePass_MoreThan10Days) {
    MockDatabase::items.push_back(Item(1, Constants::BACKSTAGE_PASS, 15, 20));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].quality, 21); // +1 when > 10 days
}

TEST_F(GildedRoseServiceTest, Settlement_BackstagePass_Exactly10Days) {
    MockDatabase::items.push_back(Item(1, Constants::BACKSTAGE_PASS, 10, 20));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].quality, 22); // +2 when 6-10 days
}

TEST_F(GildedRoseServiceTest, Settlement_BackstagePass_Exactly6Days) {
    MockDatabase::items.push_back(Item(1, Constants::BACKSTAGE_PASS, 6, 20));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].quality, 22); // +2 when 6-10 days
}

TEST_F(GildedRoseServiceTest, Settlement_BackstagePass_Exactly5Days) {
    MockDatabase::items.push_back(Item(1, Constants::BACKSTAGE_PASS, 5, 20));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].quality, 23); // +3 when 1-5 days
}

TEST_F(GildedRoseServiceTest, Settlement_BackstagePass_Exactly1Day) {
    MockDatabase::items.push_back(Item(1, Constants::BACKSTAGE_PASS, 1, 20));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].quality, 23); // +3 when 1-5 days
}

TEST_F(GildedRoseServiceTest, Settlement_AgedBrie_QualityMaxAt50) {
    MockDatabase::items.push_back(Item(1, Constants::AGED_BRIE, 2, 50));
    service.dailySettlement();
    EXPECT_LE(MockDatabase::items[0].quality, 50); // Never exceeds 50
}

TEST_F(GildedRoseServiceTest, Settlement_AgedBrie_ExpiredDoubleIncrease) {
    MockDatabase::items.push_back(Item(1, Constants::AGED_BRIE, 0, 10));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].quality, 12); // +2 when expired
}

TEST_F(GildedRoseServiceTest, Settlement_NormalItem_QualityAt1_Expired) {
    MockDatabase::items.push_back(Item(1, "Sword", 0, 1));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].quality, 0); // Would be -1, clamped to 0
}

TEST_F(GildedRoseServiceTest, Settlement_NormalItem_NegativeSellIn) {
    MockDatabase::items.push_back(Item(1, "Sword", -5, 10));
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].sellIn, -6);
    EXPECT_EQ(MockDatabase::items[0].quality, 8); // Double degradation
}

TEST_F(GildedRoseServiceTest, Settlement_TwoConsecutiveDays) {
    MockDatabase::items.push_back(Item(1, "Sword", 10, 20));
    service.dailySettlement();
    service.dailySettlement();
    EXPECT_EQ(MockDatabase::items[0].sellIn, 8);
    EXPECT_EQ(MockDatabase::items[0].quality, 18);
}

