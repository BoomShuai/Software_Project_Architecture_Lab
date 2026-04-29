#include "MockDatabase.h"

std::vector<Item> MockDatabase::items;
std::vector<User> MockDatabase::users;

void MockDatabase::init() {
    items.push_back(Item(1, "+5 Dexterity Vest", 10, 20));
    items.push_back(Item(2, "Aged Brie", 2, 0));
    items.push_back(Item(3, "Elixir of the Mongoose", 5, 7));
    items.push_back(Item(4, "Sulfuras, Hand of Ragnaros", 0, 80));
    items.push_back(Item(5, "Sulfuras, Hand of Ragnaros", -1, 80));
    items.push_back(Item(6, "Backstage passes to a TAFKAL80ETC concert", 15, 20));
    items.push_back(Item(7, "Backstage passes to a TAFKAL80ETC concert", 10, 49));
    items.push_back(Item(8, "Backstage passes to a TAFKAL80ETC concert", 5, 49));
    
    User admin;
    admin.userId = 1;
    admin.username = "admin";
    admin.password = "123456";
    admin.token = "Bearer admin_secret_token";
    users.push_back(admin);
}
