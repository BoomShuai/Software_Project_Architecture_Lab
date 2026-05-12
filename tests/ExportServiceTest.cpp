#include <gtest/gtest.h>
#include "service/ExportService.h"
#include "repository/MockDatabase.h"
#include "utils/Constants.h"

class ExportServiceTest : public ::testing::Test {
protected:
    ExportService service;
    void SetUp() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
    void TearDown() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
};

// === CSV Export Tests ===

TEST_F(ExportServiceTest, ExportItemsCSV_EmptyInventory_HeaderOnly) {
    std::string csv = service.exportAllItemsToCSV();
    EXPECT_NE(csv.find("ID,Name,SellIn,Quality"), std::string::npos);
    // Only header line, no data rows
    size_t newlines = std::count(csv.begin(), csv.end(), '\n');
    EXPECT_EQ(newlines, 1);
}

TEST_F(ExportServiceTest, ExportItemsCSV_WithItems) {
    MockDatabase::items.push_back(Item(1, "Sword", 10, 20));
    MockDatabase::items.push_back(Item(2, Constants::AGED_BRIE, 5, 30));
    std::string csv = service.exportAllItemsToCSV();
    EXPECT_NE(csv.find("Sword"), std::string::npos);
    EXPECT_NE(csv.find("Aged Brie"), std::string::npos);
    size_t newlines = std::count(csv.begin(), csv.end(), '\n');
    EXPECT_EQ(newlines, 3); // header + 2 data rows
}

TEST_F(ExportServiceTest, ExportItemsCSV_ContainsItemDto_Fields) {
    MockDatabase::items.push_back(Item(1, "Test", 10, 20));
    std::string csv = service.exportAllItemsToCSV();
    EXPECT_NE(csv.find("ITEM_1"), std::string::npos);
    EXPECT_NE(csv.find("[Gilded Rose]"), std::string::npos);
    EXPECT_NE(csv.find("days"), std::string::npos);
    EXPECT_NE(csv.find("points"), std::string::npos);
}

// === XML Export Tests ===

TEST_F(ExportServiceTest, ExportItemsXML_EmptyInventory) {
    std::string xml = service.exportAllItemsToXML();
    EXPECT_NE(xml.find("<?xml"), std::string::npos);
    EXPECT_NE(xml.find("<items>"), std::string::npos);
    EXPECT_NE(xml.find("</items>"), std::string::npos);
    EXPECT_EQ(xml.find("<item>"), std::string::npos); // No items
}

TEST_F(ExportServiceTest, ExportItemsXML_WithItems) {
    MockDatabase::items.push_back(Item(1, "Sword", 10, 20));
    std::string xml = service.exportAllItemsToXML();
    EXPECT_NE(xml.find("<item>"), std::string::npos);
    EXPECT_NE(xml.find("<name>Sword</name>"), std::string::npos);
    EXPECT_NE(xml.find("<id>1</id>"), std::string::npos);
    EXPECT_NE(xml.find("<sellIn>10</sellIn>"), std::string::npos);
    EXPECT_NE(xml.find("<quality>20</quality>"), std::string::npos);
}

TEST_F(ExportServiceTest, ExportItemsXML_MultipleItems) {
    MockDatabase::items.push_back(Item(1, "A", 1, 1));
    MockDatabase::items.push_back(Item(2, "B", 2, 2));
    std::string xml = service.exportAllItemsToXML();
    EXPECT_NE(xml.find("<name>A</name>"), std::string::npos);
    EXPECT_NE(xml.find("<name>B</name>"), std::string::npos);
}

// === User CSV Export Tests ===

TEST_F(ExportServiceTest, ExportUsersCSV_EmptyUsers_HeaderOnly) {
    std::string csv = service.exportUsersToCSV();
    EXPECT_NE(csv.find("UserId,Username,Role,Status,Contact"), std::string::npos);
    size_t newlines = std::count(csv.begin(), csv.end(), '\n');
    EXPECT_EQ(newlines, 1);
}

TEST_F(ExportServiceTest, ExportUsersCSV_WithUser) {
    User u;
    u.userId = 1;
    u.username = "admin";
    u.password = "pass";
    u.email = "admin@test.com";
    u.phone = "123";
    u.role = 1;
    u.isActive = true;
    MockDatabase::users.push_back(u);
    std::string csv = service.exportUsersToCSV();
    EXPECT_NE(csv.find("admin"), std::string::npos);
    EXPECT_NE(csv.find("ADMIN"), std::string::npos);
    EXPECT_NE(csv.find("ACTIVE"), std::string::npos);
}

TEST_F(ExportServiceTest, ExportUsersCSV_InactiveUser) {
    User u;
    u.userId = 2;
    u.username = "inactive_user";
    u.password = "pass";
    u.email = "x@y.com";
    u.phone = "000";
    u.role = 0;
    u.isActive = false;
    MockDatabase::users.push_back(u);
    std::string csv = service.exportUsersToCSV();
    EXPECT_NE(csv.find("INACTIVE"), std::string::npos);
    EXPECT_NE(csv.find("USER"), std::string::npos);
}

TEST_F(ExportServiceTest, ExportUsersCSV_ManagerRole) {
    User u;
    u.userId = 3;
    u.username = "mgr";
    u.password = "pass";
    u.email = "m@m.com";
    u.phone = "111";
    u.role = 2;
    u.isActive = true;
    MockDatabase::users.push_back(u);
    std::string csv = service.exportUsersToCSV();
    EXPECT_NE(csv.find("MANAGER"), std::string::npos);
}
