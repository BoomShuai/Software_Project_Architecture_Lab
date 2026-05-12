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

// === Exhaustive Branch Coverage ===

// quality < MIN_QUALITY (left side of ||, short-circuit true)
TEST_F(ItemServiceTest, CreateItem_QualityMinus1_Throws) {
    EXPECT_THROW(service.createItem(1, "X", 10, -1), ValidationException);
}

// quality == MIN_QUALITY (left side false, right side false -> no throw)
TEST_F(ItemServiceTest, CreateItem_QualityExactly0_Allowed) {
    EXPECT_NO_THROW(service.createItem(1, "X", 10, 0));
}

// quality == MAX_QUALITY (left side false, right side false -> no throw)
TEST_F(ItemServiceTest, CreateItem_QualityExactly50_Allowed) {
    EXPECT_NO_THROW(service.createItem(1, "X", 10, 50));
}

// quality > MAX_QUALITY (left side false, right side true -> throw)
TEST_F(ItemServiceTest, CreateItem_Quality51_Throws) {
    EXPECT_THROW(service.createItem(1, "X", 10, 51), ValidationException);
}

// quality == 1 (both sides false)
TEST_F(ItemServiceTest, CreateItem_Quality1_Allowed) {
    EXPECT_NO_THROW(service.createItem(1, "X", 10, 1));
}

// quality == 49 (both sides false)
TEST_F(ItemServiceTest, CreateItem_Quality49_Allowed) {
    EXPECT_NO_THROW(service.createItem(1, "X", 10, 49));
}

// Sulfuras bypass: name == SULFURAS && quality == 80 -> no throw
TEST_F(ItemServiceTest, CreateItem_SulfurasQuality80_Allowed) {
    EXPECT_NO_THROW(service.createItem(1, Constants::SULFURAS, 0, 80));
}

// Sulfuras bypass: name == SULFURAS && quality != 80 -> throw
TEST_F(ItemServiceTest, CreateItem_SulfurasQuality51_Throws) {
    EXPECT_THROW(service.createItem(1, Constants::SULFURAS, 0, 51), ValidationException);
}

// Sulfuras bypass: name == SULFURAS && quality == -1 -> throw
TEST_F(ItemServiceTest, CreateItem_SulfurasQualityNeg1_Throws) {
    EXPECT_THROW(service.createItem(1, Constants::SULFURAS, 0, -1), ValidationException);
}

// Non-Sulfuras name with quality > 50 -> throw (name != SULFURAS is true, short-circuit)
TEST_F(ItemServiceTest, CreateItem_NormalQuality80_Throws) {
    EXPECT_THROW(service.createItem(1, "Normal", 10, 80), ValidationException);
}

// Duplicate ID: loop with 0 existing items -> no match
TEST_F(ItemServiceTest, CreateItem_NoExisting_NoConflict) {
    EXPECT_NO_THROW(service.createItem(1, "X", 10, 20));
}

// Duplicate ID: loop with 1 existing item, different ID -> no match
TEST_F(ItemServiceTest, CreateItem_DifferentId_NoConflict) {
    service.createItem(1, "A", 10, 20);
    EXPECT_NO_THROW(service.createItem(2, "B", 10, 20));
}

// Duplicate ID: loop with 1 existing item, same ID -> match
TEST_F(ItemServiceTest, CreateItem_SameId_Throws) {
    service.createItem(1, "A", 10, 20);
    EXPECT_THROW(service.createItem(1, "B", 10, 20), ValidationException);
}

// Duplicate ID: loop with multiple items, conflict on second
TEST_F(ItemServiceTest, CreateItem_DuplicateIdOnSecond) {
    service.createItem(1, "A", 10, 20);
    service.createItem(2, "B", 10, 20);
    EXPECT_THROW(service.createItem(2, "C", 10, 20), ValidationException);
}

// getItem: empty list -> not found
TEST_F(ItemServiceTest, GetItem_EmptyList_Throws) {
    EXPECT_THROW(service.getItem(1), ItemNotFoundException);
}

// getItem: single item, match
TEST_F(ItemServiceTest, GetItem_SingleMatch) {
    service.createItem(1, "X", 10, 20);
    EXPECT_NO_THROW(service.getItem(1));
}

// getItem: single item, no match
TEST_F(ItemServiceTest, GetItem_SingleNoMatch) {
    service.createItem(1, "X", 10, 20);
    EXPECT_THROW(service.getItem(2), ItemNotFoundException);
}

// getItem: multiple items, match on last
TEST_F(ItemServiceTest, GetItem_MatchOnThird) {
    service.createItem(1, "A", 10, 10);
    service.createItem(2, "B", 10, 10);
    service.createItem(3, "C", 10, 10);
    Item result = service.getItem(3);
    EXPECT_EQ(result.name, "C");
}

// updateItem: empty list -> not found
TEST_F(ItemServiceTest, UpdateItem_EmptyList_Throws) {
    EXPECT_THROW(service.updateItem(1, "X", 1, 1), ItemNotFoundException);
}

// updateItem: match on first item
TEST_F(ItemServiceTest, UpdateItem_MatchOnFirst) {
    service.createItem(1, "A", 10, 20);
    service.createItem(2, "B", 10, 20);
    service.updateItem(1, "AA", 5, 5);
    EXPECT_EQ(MockDatabase::items[0].name, "AA");
}

// updateItem: match on second item
TEST_F(ItemServiceTest, UpdateItem_MatchOnSecond) {
    service.createItem(1, "A", 10, 20);
    service.createItem(2, "B", 10, 20);
    service.updateItem(2, "BB", 5, 5);
    EXPECT_EQ(MockDatabase::items[1].name, "BB");
}

// deleteItem: empty list -> no crash
TEST_F(ItemServiceTest, DeleteItem_EmptyList_NoThrow) {
    EXPECT_NO_THROW(service.deleteItem(999));
}

// deleteItem: single item match
TEST_F(ItemServiceTest, DeleteItem_SingleMatch) {
    service.createItem(1, "X", 10, 20);
    service.deleteItem(1);
    EXPECT_EQ(MockDatabase::items.size(), 0);
}

// deleteItem: single item no match
TEST_F(ItemServiceTest, DeleteItem_SingleNoMatch) {
    service.createItem(1, "X", 10, 20);
    service.deleteItem(2);
    EXPECT_EQ(MockDatabase::items.size(), 1);
}

// deleteItem: multiple items, delete middle
TEST_F(ItemServiceTest, DeleteItem_Middle) {
    service.createItem(1, "A", 10, 10);
    service.createItem(2, "B", 10, 10);
    service.createItem(3, "C", 10, 10);
    service.deleteItem(2);
    EXPECT_EQ(MockDatabase::items.size(), 2);
}

// deleteItem: multiple items, delete first
TEST_F(ItemServiceTest, DeleteItem_First) {
    service.createItem(1, "A", 10, 10);
    service.createItem(2, "B", 10, 10);
    service.deleteItem(1);
    EXPECT_EQ(MockDatabase::items.size(), 1);
    EXPECT_EQ(MockDatabase::items[0].name, "B");
}

// deleteItem: multiple items, delete last
TEST_F(ItemServiceTest, DeleteItem_Last) {
    service.createItem(1, "A", 10, 10);
    service.createItem(2, "B", 10, 10);
    service.deleteItem(2);
    EXPECT_EQ(MockDatabase::items.size(), 1);
    EXPECT_EQ(MockDatabase::items[0].name, "A");
}

// checkInventoryWarning: empty list
TEST_F(ItemServiceTest, CheckWarning_EmptyList) {
    auto w = service.checkInventoryWarning();
    EXPECT_EQ(w.size(), 0);
}

// checkInventoryWarning: sellIn < threshold only
TEST_F(ItemServiceTest, CheckWarning_LowSellInOnly) {
    service.createItem(1, "X", 1, 30);
    auto w = service.checkInventoryWarning();
    EXPECT_EQ(w.size(), 1);
}

// checkInventoryWarning: quality < threshold only
TEST_F(ItemServiceTest, CheckWarning_LowQualityOnly) {
    service.createItem(1, "X", 30, 2);
    auto w = service.checkInventoryWarning();
    EXPECT_EQ(w.size(), 1);
}

// checkInventoryWarning: both low
TEST_F(ItemServiceTest, CheckWarning_BothLow) {
    service.createItem(1, "X", 1, 2);
    auto w = service.checkInventoryWarning();
    EXPECT_EQ(w.size(), 1);
}

// checkInventoryWarning: neither low
TEST_F(ItemServiceTest, CheckWarning_NeitherLow) {
    service.createItem(1, "X", 30, 30);
    auto w = service.checkInventoryWarning();
    EXPECT_EQ(w.size(), 0);
}

// checkInventoryWarning: exactly at threshold values (boundary)
TEST_F(ItemServiceTest, CheckWarning_ExactThreshold_NotFlagged) {
    service.createItem(1, "X", Constants::SELL_IN_WARNING, Constants::QUALITY_WARNING);
    auto w = service.checkInventoryWarning();
    EXPECT_EQ(w.size(), 0);
}

// checkInventoryWarning: just below threshold
TEST_F(ItemServiceTest, CheckWarning_JustBelowSellInThreshold) {
    service.createItem(1, "X", Constants::SELL_IN_WARNING - 1, 30);
    auto w = service.checkInventoryWarning();
    EXPECT_EQ(w.size(), 1);
}

TEST_F(ItemServiceTest, CheckWarning_JustBelowQualityThreshold) {
    service.createItem(1, "X", 30, Constants::QUALITY_WARNING - 1);
    auto w = service.checkInventoryWarning();
    EXPECT_EQ(w.size(), 1);
}

// checkInventoryWarning: multiple items, mixed
TEST_F(ItemServiceTest, CheckWarning_MultiMixed) {
    service.createItem(1, "Healthy", 30, 30);
    service.createItem(2, "LowSellIn", 1, 30);
    service.createItem(3, "LowQuality", 30, 1);
    service.createItem(4, "AlsoHealthy", 20, 20);
    auto w = service.checkInventoryWarning();
    EXPECT_EQ(w.size(), 2);
}

// getAllItems: single item
TEST_F(ItemServiceTest, GetAllItems_SingleItem) {
    service.createItem(1, "X", 10, 20);
    auto all = service.getAllItems();
    EXPECT_EQ(all.size(), 1);
}

// getAllItems: multiple items
TEST_F(ItemServiceTest, GetAllItems_MultipleItems) {
    service.createItem(1, "A", 10, 20);
    service.createItem(2, "B", 5, 10);
    service.createItem(3, "C", 1, 1);
    auto all = service.getAllItems();
    EXPECT_EQ(all.size(), 3);
}


