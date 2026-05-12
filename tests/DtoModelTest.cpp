#include <gtest/gtest.h>
#include "dto/ItemDto.h"
#include "dto/UserDto.h"
#include "model/Item.h"
#include "model/User.h"

// === ItemDto Tests ===

TEST(ItemDtoTest, FromEntity_BasicFields) {
    Item item(1, "Sword", 10, 20);
    ItemDto dto = ItemDto::fromEntity(item);
    EXPECT_EQ(dto.itemIdStr, "ITEM_1");
    EXPECT_EQ(dto.displayName, "[Gilded Rose] Sword");
    EXPECT_EQ(dto.daysRemaining, "10 days");
    EXPECT_EQ(dto.itemQuality, "20 points");
}

TEST(ItemDtoTest, FromEntity_DescriptionAndCategory) {
    Item item(2, "Brie", 5, 30, "Aged cheese", "Food");
    ItemDto dto = ItemDto::fromEntity(item);
    EXPECT_NE(dto.fullDescription.find("Food"), std::string::npos);
    EXPECT_NE(dto.fullDescription.find("Aged cheese"), std::string::npos);
}

TEST(ItemDtoTest, FromEntity_AuditInfo) {
    Item item(3, "Test", 1, 1);
    ItemDto dto = ItemDto::fromEntity(item);
    EXPECT_NE(dto.auditInfo.find("Created:"), std::string::npos);
    EXPECT_NE(dto.auditInfo.find("By:"), std::string::npos);
}

TEST(ItemDtoTest, FromEntity_LargeId) {
    Item item(99999, "BigId", 1, 1);
    ItemDto dto = ItemDto::fromEntity(item);
    EXPECT_EQ(dto.itemIdStr, "ITEM_99999");
}

TEST(ItemDtoTest, FromEntity_ZeroValues) {
    Item item(0, "Zero", 0, 0);
    ItemDto dto = ItemDto::fromEntity(item);
    EXPECT_EQ(dto.daysRemaining, "0 days");
    EXPECT_EQ(dto.itemQuality, "0 points");
}

// === UserDto Tests ===

TEST(UserDtoTest, FromEntity_AdminRole) {
    User u;
    u.userId = 1;
    u.username = "admin";
    u.email = "admin@test.com";
    u.phone = "12345";
    u.role = 1;
    u.isActive = true;
    UserDto dto = UserDto::fromEntity(u);
    EXPECT_EQ(dto.publicUserId, "U_1");
    EXPECT_EQ(dto.displayUsername, "admin");
    EXPECT_EQ(dto.roleName, "ADMIN");
    EXPECT_EQ(dto.status, "ACTIVE");
    EXPECT_NE(dto.contactInfo.find("admin@test.com"), std::string::npos);
    EXPECT_NE(dto.contactInfo.find("12345"), std::string::npos);
}

TEST(UserDtoTest, FromEntity_ManagerRole) {
    User u;
    u.userId = 2;
    u.username = "mgr";
    u.email = "m@m.com";
    u.phone = "111";
    u.role = 2;
    u.isActive = true;
    UserDto dto = UserDto::fromEntity(u);
    EXPECT_EQ(dto.roleName, "MANAGER");
}

TEST(UserDtoTest, FromEntity_UserRole) {
    User u;
    u.userId = 3;
    u.username = "user";
    u.email = "u@u.com";
    u.phone = "222";
    u.role = 0;
    u.isActive = true;
    UserDto dto = UserDto::fromEntity(u);
    EXPECT_EQ(dto.roleName, "USER");
}

TEST(UserDtoTest, FromEntity_UnknownRole_DefaultsToUser) {
    User u;
    u.userId = 4;
    u.username = "x";
    u.email = "x@x.com";
    u.phone = "333";
    u.role = 99;
    u.isActive = false;
    UserDto dto = UserDto::fromEntity(u);
    EXPECT_EQ(dto.roleName, "USER");
    EXPECT_EQ(dto.status, "INACTIVE");
}

TEST(UserDtoTest, FromEntity_InactiveUser) {
    User u;
    u.userId = 5;
    u.username = "inactive";
    u.email = "i@i.com";
    u.phone = "444";
    u.role = 1;
    u.isActive = false;
    UserDto dto = UserDto::fromEntity(u);
    EXPECT_EQ(dto.status, "INACTIVE");
}

// === Item Model Tests ===

TEST(ItemModelTest, DefaultConstructor) {
    Item item;
    EXPECT_EQ(item.id, 0);
    EXPECT_EQ(item.name, "");
    EXPECT_EQ(item.sellIn, 0);
    EXPECT_EQ(item.quality, 0);
}

TEST(ItemModelTest, FourArgConstructor) {
    Item item(1, "Sword", 10, 20);
    EXPECT_EQ(item.id, 1);
    EXPECT_EQ(item.name, "Sword");
    EXPECT_EQ(item.sellIn, 10);
    EXPECT_EQ(item.quality, 20);
    EXPECT_EQ(item.description, "Default item");
    EXPECT_EQ(item.category, "General");
}

TEST(ItemModelTest, SixArgConstructor) {
    Item item(1, "Sword", 10, 20, "Sharp blade", "Weapon");
    EXPECT_EQ(item.description, "Sharp blade");
    EXPECT_EQ(item.category, "Weapon");
}

// === User Model Tests ===

TEST(UserModelTest, DefaultConstructor) {
    User u;
    EXPECT_EQ(u.userId, 0);
    EXPECT_EQ(u.username, "");
    EXPECT_EQ(u.password, "");
    EXPECT_EQ(u.token, "");
    EXPECT_EQ(u.role, 0);
    EXPECT_FALSE(u.isActive);
}
