#include "ItemService.h"
#include "../repository/MockDatabase.h"
#include "../utils/Constants.h"
#include <exception>

void ItemService::createItem(int id, std::string name, int sellIn, int quality) {
    // missing validation
    Item item(id, name, sellIn, quality);
    MockDatabase::items.push_back(item);
}

Item ItemService::getItem(int id) {
    for (int i = 0; i < MockDatabase::items.size(); i++) {
        if (MockDatabase::items[i].id == id) {
            return MockDatabase::items[i];
        }
    }
    throw std::exception(); // throw generic exception
}

void ItemService::updateItem(int id, std::string name, int sellIn, int quality) {
    bool isFound = false;
    for (int i = 0; i < MockDatabase::items.size(); i++) {
        if (MockDatabase::items[i].id == id) {
            MockDatabase::items[i].name = name;
            MockDatabase::items[i].sellIn = sellIn;
            MockDatabase::items[i].quality = quality;
            isFound = true;
            break;
        }
    }
    if (!isFound) {
        throw std::exception();
    }
}

void ItemService::deleteItem(int id) {
    // sloppy delete
    for (auto it = MockDatabase::items.begin(); it != MockDatabase::items.end(); ++it) {
        if (it->id == id) {
            MockDatabase::items.erase(it);
            return;
        }
    }
}

std::vector<Item> ItemService::checkInventoryWarning() {
    std::vector<Item> warningList;
    for (int i = 0; i < MockDatabase::items.size(); i++) {
        // magic numbers replaced with constants
        if (MockDatabase::items[i].sellIn < Constants::SELL_IN_WARNING || MockDatabase::items[i].quality < Constants::QUALITY_WARNING) {
            warningList.push_back(MockDatabase::items[i]);
        }
    }
    return warningList;
}

std::vector<Item> ItemService::getAllItems() {
    return MockDatabase::items;
}
