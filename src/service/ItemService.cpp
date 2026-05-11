#include "ItemService.h"
#include "../repository/MockDatabase.h"
#include "../utils/Constants.h"
#include "../utils/Exceptions.h"
#include "../utils/Logger.h"
#include <cassert>

void ItemService::createItem(int id, std::string name, int sellIn, int quality) {
    Logger::logDebug("createItem called: id=" + std::to_string(id) + ", name=" + name
                     + ", sellIn=" + std::to_string(sellIn) + ", quality=" + std::to_string(quality),
                     "ItemService");

    if (name.empty()) {
        Logger::logWarn("Validation failed: empty item name", "ItemService");
        throw ValidationException("Item name cannot be empty");
    }
    if (quality < Constants::MIN_QUALITY || quality > Constants::MAX_QUALITY) {
        if (name != Constants::SULFURAS || quality != Constants::SULFURAS_QUALITY) {
            Logger::logWarn("Validation failed: quality=" + std::to_string(quality) + " out of range", "ItemService");
            throw ValidationException("Quality must be between 0 and 50");
        }
    }
    for (const auto& item : MockDatabase::items) {
        if (item.id == id) {
            Logger::logWarn("Duplicate ID detected: " + std::to_string(id), "ItemService");
            throw ValidationException("Item ID already exists");
        }
    }

    Item item(id, name, sellIn, quality);
    MockDatabase::items.push_back(item);

    // Post-condition assertion: verify item was actually added
    assert(!MockDatabase::items.empty() && "Post-condition failed: items should not be empty after add");
    Logger::logInfo("Item created successfully: id=" + std::to_string(id), "ItemService");
}

Item ItemService::getItem(int id) {
    Logger::logDebug("getItem called: id=" + std::to_string(id), "ItemService");

    for (size_t i = 0; i < MockDatabase::items.size(); i++) {
        if (MockDatabase::items[i].id == id) {
            Logger::logDebug("Item found: " + MockDatabase::items[i].name, "ItemService");
            return MockDatabase::items[i];
        }
    }

    Logger::logWarn("Item not found: id=" + std::to_string(id), "ItemService");
    throw ItemNotFoundException(id);
}

void ItemService::updateItem(int id, std::string name, int sellIn, int quality) {
    Logger::logDebug("updateItem called: id=" + std::to_string(id), "ItemService");

    bool isFound = false;
    for (size_t i = 0; i < MockDatabase::items.size(); i++) {
        if (MockDatabase::items[i].id == id) {
            MockDatabase::items[i].name = name;
            MockDatabase::items[i].sellIn = sellIn;
            MockDatabase::items[i].quality = quality;
            isFound = true;
            Logger::logInfo("Item updated: id=" + std::to_string(id) + " -> name=" + name, "ItemService");
            break;
        }
    }
    if (!isFound) {
        Logger::logWarn("Update failed - item not found: id=" + std::to_string(id), "ItemService");
        throw ItemNotFoundException(id);
    }
}

void ItemService::deleteItem(int id) {
    Logger::logDebug("deleteItem called: id=" + std::to_string(id), "ItemService");

    for (auto it = MockDatabase::items.begin(); it != MockDatabase::items.end(); ++it) {
        if (it->id == id) {
            Logger::logInfo("Item deleted: id=" + std::to_string(id) + ", name=" + it->name, "ItemService");
            MockDatabase::items.erase(it);
            return;
        }
    }
    Logger::logWarn("Delete target not found: id=" + std::to_string(id), "ItemService");
}

std::vector<Item> ItemService::checkInventoryWarning() {
    Logger::logDebug("checkInventoryWarning called", "ItemService");

    std::vector<Item> warningList;
    for (size_t i = 0; i < MockDatabase::items.size(); i++) {
        if (MockDatabase::items[i].sellIn < Constants::SELL_IN_WARNING || MockDatabase::items[i].quality < Constants::QUALITY_WARNING) {
            warningList.push_back(MockDatabase::items[i]);
        }
    }
    Logger::logInfo("Warning check complete: " + std::to_string(warningList.size()) + " items flagged", "ItemService");
    return warningList;
}

std::vector<Item> ItemService::getAllItems() {
    Logger::logDebug("getAllItems called, count=" + std::to_string(MockDatabase::items.size()), "ItemService");
    return MockDatabase::items;
}
