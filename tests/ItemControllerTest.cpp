#include <gtest/gtest.h>
#include "controller/ItemController.h"
#include "repository/MockDatabase.h"
#include "utils/Constants.h"
#include "utils/Exceptions.h"

class ItemControllerTest : public ::testing::Test {
protected:
    ItemController controller;
    void SetUp() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
    void TearDown() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
};

// === handleAddItemRequest ===

TEST_F(ItemControllerTest, AddItem_HappyPath) {
    EXPECT_NO_THROW(controller.handleAddItemRequest(1, "Sword", 10, 20));
    EXPECT_EQ(MockDatabase::items.size(), 1);
}

TEST_F(ItemControllerTest, AddItem_MultipleItems) {
    controller.handleAddItemRequest(1, "Sword", 10, 20);
    controller.handleAddItemRequest(2, "Shield", 5, 15);
    EXPECT_EQ(MockDatabase::items.size(), 2);
}

// === handleGetItemRequest ===

TEST_F(ItemControllerTest, GetItem_HappyPath) {
    MockDatabase::items.push_back(Item(1, "Sword", 10, 20));
    EXPECT_NO_THROW(controller.handleGetItemRequest(1));
}

TEST_F(ItemControllerTest, GetItem_NotFound) {
    EXPECT_THROW(controller.handleGetItemRequest(999), ItemNotFoundException);
}

// === handleUpdateItemRequest ===

TEST_F(ItemControllerTest, UpdateItem_HappyPath) {
    MockDatabase::items.push_back(Item(1, "Sword", 10, 20));
    EXPECT_NO_THROW(controller.handleUpdateItemRequest(1, "Gold Sword", 15, 30));
    EXPECT_EQ(MockDatabase::items[0].name, "Gold Sword");
}

TEST_F(ItemControllerTest, UpdateItem_NotFound) {
    EXPECT_THROW(controller.handleUpdateItemRequest(999, "X", 1, 1), ItemNotFoundException);
}

// === handleDeleteItemRequest ===

TEST_F(ItemControllerTest, DeleteItem_HappyPath) {
    MockDatabase::items.push_back(Item(1, "Sword", 10, 20));
    EXPECT_NO_THROW(controller.handleDeleteItemRequest(1));
    EXPECT_EQ(MockDatabase::items.size(), 0);
}

TEST_F(ItemControllerTest, DeleteItem_NonExistent_NoThrow) {
    EXPECT_NO_THROW(controller.handleDeleteItemRequest(999));
}

// === handleDailySettlementRequest ===

TEST_F(ItemControllerTest, DailySettlement_HappyPath) {
    MockDatabase::items.push_back(Item(1, "Sword", 10, 20));
    EXPECT_NO_THROW(controller.handleDailySettlementRequest());
    EXPECT_EQ(MockDatabase::items[0].quality, 19);
}

TEST_F(ItemControllerTest, DailySettlement_EmptyInventory) {
    EXPECT_NO_THROW(controller.handleDailySettlementRequest());
}

// === handleInventoryWarningRequest ===

TEST_F(ItemControllerTest, InventoryWarning_NoWarnings) {
    MockDatabase::items.push_back(Item(1, "Sword", 30, 40));
    EXPECT_NO_THROW(controller.handleInventoryWarningRequest());
}

TEST_F(ItemControllerTest, InventoryWarning_WithWarnings) {
    MockDatabase::items.push_back(Item(1, "Expiring", 2, 3));
    EXPECT_NO_THROW(controller.handleInventoryWarningRequest());
}
