/**
 * @file ItemStrategyTest.cpp
 * @brief Unit tests for the Strategy Pattern (core business logic).
 *
 * Tests all four item update strategies: Normal, Aged Brie, Sulfuras, Backstage Pass.
 * Focuses on:
 * - Happy path behavior
 * - Boundary conditions (quality 0, quality 50, sellIn 0)
 * - Edge cases AI code typically misses (negative sellIn, double degradation)
 */

#include <gtest/gtest.h>
#include "model/Item.h"
#include "service/strategy/ItemStrategy.h"
#include "utils/Constants.h"
#include <memory>

// ===================================================================
// Normal Item Strategy Tests
// ===================================================================
class NormalItemStrategyTest : public ::testing::Test {
protected:
    NormalItemStrategy strategy;
};

TEST_F(NormalItemStrategyTest, QualityDecreasesBy1BeforeSellDate) {
    Item item(1, "Normal Sword", 10, 20);
    strategy.update(item);
    EXPECT_EQ(item.quality, 19);
    EXPECT_EQ(item.sellIn, 9);
}

TEST_F(NormalItemStrategyTest, QualityDecreasesBy2AfterSellDate) {
    Item item(1, "Normal Sword", 0, 20);
    strategy.update(item);
    // sellIn goes to -1, quality decreases by 1 + 1 (double degradation)
    EXPECT_EQ(item.quality, 18);
    EXPECT_EQ(item.sellIn, -1);
}

TEST_F(NormalItemStrategyTest, QualityNeverGoesNegative) {
    Item item(1, "Normal Sword", 5, 0);
    strategy.update(item);
    EXPECT_EQ(item.quality, 0);
}

TEST_F(NormalItemStrategyTest, QualityNeverGoesNegativeAfterSellDate) {
    Item item(1, "Normal Sword", -1, 1);
    strategy.update(item);
    // Would decrease by 2, but clamped at 0
    EXPECT_EQ(item.quality, 0);
}

TEST_F(NormalItemStrategyTest, SellInAlwaysDecreases) {
    Item item(1, "Normal Sword", -5, 10);
    strategy.update(item);
    EXPECT_EQ(item.sellIn, -6);
}

// ===================================================================
// Aged Brie Strategy Tests
// ===================================================================
class AgedBrieStrategyTest : public ::testing::Test {
protected:
    AgedBrieStrategy strategy;
};

TEST_F(AgedBrieStrategyTest, QualityIncreasesBy1BeforeSellDate) {
    Item item(2, "Aged Brie", 5, 10);
    strategy.update(item);
    EXPECT_EQ(item.quality, 11);
    EXPECT_EQ(item.sellIn, 4);
}

TEST_F(AgedBrieStrategyTest, QualityIncreasesBy2AfterSellDate) {
    Item item(2, "Aged Brie", 0, 10);
    strategy.update(item);
    EXPECT_EQ(item.quality, 12);
    EXPECT_EQ(item.sellIn, -1);
}

TEST_F(AgedBrieStrategyTest, QualityNeverExceeds50) {
    Item item(2, "Aged Brie", 5, 50);
    strategy.update(item);
    EXPECT_EQ(item.quality, 50);
}

TEST_F(AgedBrieStrategyTest, QualityNeverExceeds50AfterSellDate) {
    Item item(2, "Aged Brie", -1, 49);
    strategy.update(item);
    // Would go 49 + 1 + 1 = 51, clamped at 50
    EXPECT_EQ(item.quality, 50);
}

TEST_F(AgedBrieStrategyTest, QualityStartsFromZero) {
    Item item(2, "Aged Brie", 10, 0);
    strategy.update(item);
    EXPECT_EQ(item.quality, 1);
}

// ===================================================================
// Sulfuras Strategy Tests
// ===================================================================
class SulfurasStrategyTest : public ::testing::Test {
protected:
    SulfurasStrategy strategy;
};

TEST_F(SulfurasStrategyTest, QualityNeverChanges) {
    Item item(4, Constants::SULFURAS, 0, 80);
    strategy.update(item);
    EXPECT_EQ(item.quality, 80);
}

TEST_F(SulfurasStrategyTest, SellInNeverChanges) {
    Item item(4, Constants::SULFURAS, 10, 80);
    strategy.update(item);
    EXPECT_EQ(item.sellIn, 10);
}

TEST_F(SulfurasStrategyTest, NegativeSellInStaysUnchanged) {
    Item item(5, Constants::SULFURAS, -1, 80);
    strategy.update(item);
    EXPECT_EQ(item.sellIn, -1);
    EXPECT_EQ(item.quality, 80);
}

// ===================================================================
// Backstage Pass Strategy Tests
// ===================================================================
class BackstagePassStrategyTest : public ::testing::Test {
protected:
    BackstagePassStrategy strategy;
};

TEST_F(BackstagePassStrategyTest, QualityIncreasesBy1WhenMoreThan10Days) {
    Item item(6, Constants::BACKSTAGE_PASS, 15, 20);
    strategy.update(item);
    EXPECT_EQ(item.quality, 21);
    EXPECT_EQ(item.sellIn, 14);
}

TEST_F(BackstagePassStrategyTest, QualityIncreasesBy2When10DaysOrLess) {
    Item item(6, Constants::BACKSTAGE_PASS, 10, 20);
    strategy.update(item);
    // sellIn becomes 9, which is < 11, so +1 +1 = +2
    EXPECT_EQ(item.quality, 22);
}

TEST_F(BackstagePassStrategyTest, QualityIncreasesBy3When5DaysOrLess) {
    Item item(6, Constants::BACKSTAGE_PASS, 5, 20);
    strategy.update(item);
    // sellIn becomes 4, which is < 11 and < 6, so +1 +1 +1 = +3
    EXPECT_EQ(item.quality, 23);
}

TEST_F(BackstagePassStrategyTest, QualityDropsToZeroAfterConcert) {
    Item item(6, Constants::BACKSTAGE_PASS, 0, 50);
    strategy.update(item);
    // sellIn becomes -1, quality drops to 0
    EXPECT_EQ(item.quality, 0);
    EXPECT_EQ(item.sellIn, -1);
}

TEST_F(BackstagePassStrategyTest, QualityNeverExceeds50) {
    Item item(6, Constants::BACKSTAGE_PASS, 3, 49);
    strategy.update(item);
    // Would go 49 + 3 = 52, clamped at 50
    EXPECT_EQ(item.quality, 50);
}

// ===================================================================
// Strategy Factory Tests
// ===================================================================
TEST(ItemStrategyFactoryTest, CreatesNormalStrategyForUnknownItem) {
    auto* strategy = ItemStrategyFactory::createStrategy("Unknown Item");
    ASSERT_NE(strategy, nullptr);
    auto* normalStrategy = dynamic_cast<NormalItemStrategy*>(strategy);
    EXPECT_NE(normalStrategy, nullptr);
    delete strategy;
}

TEST(ItemStrategyFactoryTest, CreatesAgedBrieStrategy) {
    auto* strategy = ItemStrategyFactory::createStrategy(Constants::AGED_BRIE);
    ASSERT_NE(strategy, nullptr);
    auto* agedBrieStrategy = dynamic_cast<AgedBrieStrategy*>(strategy);
    EXPECT_NE(agedBrieStrategy, nullptr);
    delete strategy;
}

TEST(ItemStrategyFactoryTest, CreatesSulfurasStrategy) {
    auto* strategy = ItemStrategyFactory::createStrategy(Constants::SULFURAS);
    auto* sulfurasStrategy = dynamic_cast<SulfurasStrategy*>(strategy);
    EXPECT_NE(sulfurasStrategy, nullptr);
    delete strategy;
}

TEST(ItemStrategyFactoryTest, CreatesBackstagePassStrategy) {
    auto* strategy = ItemStrategyFactory::createStrategy(Constants::BACKSTAGE_PASS);
    auto* backstageStrategy = dynamic_cast<BackstagePassStrategy*>(strategy);
    EXPECT_NE(backstageStrategy, nullptr);
    delete strategy;
}
