/**
 * @file ItemServiceTest.cpp
 * @brief Unit tests for ItemService CRUD operations.
 *
 * Each test initializes a clean MockDatabase state via SetUp/TearDown
 * to ensure test isolation — a key testability improvement.
 *
 * Tests cover:
 * - CRUD happy paths
 * - Validation boundary conditions (empty name, out-of-range quality, duplicate ID)
 * - Exception path testing (AI code often only tests happy paths)
 */

#include <gtest/gtest.h>
#include "service/ItemService.h"
#include "repository/MockDatabase.h"
#include "utils/Exceptions.h"
#include "utils/Constants.h"

class ItemServiceTest : public ::testing::Test {
protected:
    ItemService service;

    void SetUp() override {
        // Start each test with a clean slate
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }

    void TearDown() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
};

// === CREATE ===
TEST_F(ItemServiceTest, CreateItem_HappyPath) {
    service.createItem(1, "Test Sword", 10, 20);
    ASSERT_EQ(MockDatabase::items.size(), 1);
    EXPECT_EQ(MockDatabase::items[0].name, "Test Sword");
    EXPECT_EQ(MockDatabase::items[0].sellIn, 10);
    EXPECT_EQ(MockDatabase::items[0].quality, 20);
}

TEST_F(ItemServiceTest, CreateItem_ThrowsOnEmptyName) {
    EXPECT_THROW(service.createItem(1, "", 10, 20), ValidationException);
}

TEST_F(ItemServiceTest, CreateItem_ThrowsOnQualityTooHigh) {
    EXPECT_THROW(service.createItem(1, "Sword", 10, 51), ValidationException);
}

TEST_F(ItemServiceTest, CreateItem_ThrowsOnNegativeQuality) {
    EXPECT_THROW(service.createItem(1, "Sword", 10, -1), ValidationException);
}

TEST_F(ItemServiceTest, CreateItem_ThrowsOnDuplicateId) {
    service.createItem(1, "Sword", 10, 20);
    EXPECT_THROW(service.createItem(1, "Shield", 5, 10), ValidationException);
}

TEST_F(ItemServiceTest, CreateItem_SulfurasAllows80Quality) {
    EXPECT_NO_THROW(service.createItem(1, Constants::SULFURAS, 0, 80));
    ASSERT_EQ(MockDatabase::items.size(), 1);
    EXPECT_EQ(MockDatabase::items[0].quality, 80);
}

// === READ ===
TEST_F(ItemServiceTest, GetItem_HappyPath) {
    service.createItem(42, "Magic Ring", 7, 30);
    Item result = service.getItem(42);
    EXPECT_EQ(result.name, "Magic Ring");
    EXPECT_EQ(result.quality, 30);
}

TEST_F(ItemServiceTest, GetItem_ThrowsOnNotFound) {
    EXPECT_THROW(service.getItem(999), ItemNotFoundException);
}

// === UPDATE ===
TEST_F(ItemServiceTest, UpdateItem_HappyPath) {
    service.createItem(1, "Old Name", 10, 20);
    service.updateItem(1, "New Name", 5, 30);

    Item result = service.getItem(1);
    EXPECT_EQ(result.name, "New Name");
    EXPECT_EQ(result.sellIn, 5);
    EXPECT_EQ(result.quality, 30);
}

TEST_F(ItemServiceTest, UpdateItem_ThrowsOnNotFound) {
    EXPECT_THROW(service.updateItem(999, "Name", 1, 1), ItemNotFoundException);
}

// === DELETE ===
TEST_F(ItemServiceTest, DeleteItem_HappyPath) {
    service.createItem(1, "To Delete", 10, 20);
    ASSERT_EQ(MockDatabase::items.size(), 1);
    service.deleteItem(1);
    EXPECT_EQ(MockDatabase::items.size(), 0);
}

TEST_F(ItemServiceTest, DeleteItem_NonexistentIdDoesNotThrow) {
    // Current implementation silently does nothing — test documents this behavior
    EXPECT_NO_THROW(service.deleteItem(999));
}

// === INVENTORY WARNING ===
TEST_F(ItemServiceTest, CheckInventoryWarning_FlagsLowSellIn) {
    service.createItem(1, "Expiring Item", 2, 30);  // sellIn < SELL_IN_WARNING(3)
    auto warnings = service.checkInventoryWarning();
    EXPECT_EQ(warnings.size(), 1);
}

TEST_F(ItemServiceTest, CheckInventoryWarning_FlagsLowQuality) {
    service.createItem(1, "Bad Quality", 10, 3);  // quality < QUALITY_WARNING(5)
    auto warnings = service.checkInventoryWarning();
    EXPECT_EQ(warnings.size(), 1);
}

TEST_F(ItemServiceTest, CheckInventoryWarning_EmptyWhenAllHealthy) {
    service.createItem(1, "Healthy Item", 10, 30);
    auto warnings = service.checkInventoryWarning();
    EXPECT_EQ(warnings.size(), 0);
}

// === GET ALL ===
TEST_F(ItemServiceTest, GetAllItems_ReturnsAllItems) {
    service.createItem(1, "A", 10, 20);
    service.createItem(2, "B", 5, 10);
    auto all = service.getAllItems();
    EXPECT_EQ(all.size(), 2);
}

TEST_F(ItemServiceTest, GetAllItems_EmptyWhenNoItems) {
    auto all = service.getAllItems();
    EXPECT_EQ(all.size(), 0);
}

// === Boundary and Branch Coverage ===

TEST_F(ItemServiceTest, CreateItem_QualityExactly0_Allowed) {
    EXPECT_NO_THROW(service.createItem(1, "Zero Quality", 10, 0));
    EXPECT_EQ(MockDatabase::items[0].quality, 0);
}

TEST_F(ItemServiceTest, CreateItem_QualityExactly50_Allowed) {
    EXPECT_NO_THROW(service.createItem(1, "Max Quality", 10, 50));
    EXPECT_EQ(MockDatabase::items[0].quality, 50);
}

TEST_F(ItemServiceTest, CreateItem_SulfurasNon80Quality_Throws) {
    // Sulfuras with quality != 80 should throw
    EXPECT_THROW(service.createItem(1, Constants::SULFURAS, 0, 51), ValidationException);
}

TEST_F(ItemServiceTest, CreateItem_SulfurasNegativeQuality_Throws) {
    EXPECT_THROW(service.createItem(1, Constants::SULFURAS, 0, -1), ValidationException);
}

TEST_F(ItemServiceTest, CreateItem_NormalItemQuality80_Throws) {
    // Normal item with quality 80 should throw (only Sulfuras allowed)
    EXPECT_THROW(service.createItem(1, "Normal", 10, 80), ValidationException);
}

TEST_F(ItemServiceTest, GetItem_FindsSecondItem) {
    service.createItem(1, "First", 10, 20);
    service.createItem(2, "Second", 5, 15);
    Item result = service.getItem(2);
    EXPECT_EQ(result.name, "Second");
}

TEST_F(ItemServiceTest, UpdateItem_VerifiesOldDataReplaced) {
    service.createItem(1, "Old", 10, 20);
    service.updateItem(1, "New", 99, 49);
    EXPECT_EQ(MockDatabase::items[0].name, "New");
    EXPECT_EQ(MockDatabase::items[0].sellIn, 99);
    EXPECT_EQ(MockDatabase::items[0].quality, 49);
}

TEST_F(ItemServiceTest, DeleteItem_MiddleItem) {
    service.createItem(1, "A", 10, 10);
    service.createItem(2, "B", 10, 10);
    service.createItem(3, "C", 10, 10);
    service.deleteItem(2);
    EXPECT_EQ(MockDatabase::items.size(), 2);
    EXPECT_EQ(MockDatabase::items[0].name, "A");
    EXPECT_EQ(MockDatabase::items[1].name, "C");
}

TEST_F(ItemServiceTest, CheckInventoryWarning_BoundaryNotFlagged) {
    // sellIn exactly at warning threshold, quality above warning
    service.createItem(1, "OnBorder", Constants::SELL_IN_WARNING, Constants::QUALITY_WARNING);
    auto warnings = service.checkInventoryWarning();
    EXPECT_EQ(warnings.size(), 0);
}

