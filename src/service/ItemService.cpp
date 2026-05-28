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

    // OPTIMIZED: O(1) duplicate check using HashMap index (was O(n) linear scan)
    if (MockDatabase::itemIndex.find(id) != MockDatabase::itemIndex.end()) {
        Logger::logWarn("Duplicate ID detected: " + std::to_string(id), "ItemService");
        throw ValidationException("Item ID already exists");
    }

    Item item(id, name, sellIn, quality);
    MockDatabase::items.push_back(item);
    MockDatabase::rebuildIndex(); // Maintain index consistency

    // Post-condition assertion: verify item was actually added
    assert(!MockDatabase::items.empty() && "Post-condition failed: items should not be empty after add");

    // OPTIMIZED: Pre-populate cache with new item
    itemCache.put(id, item);

    Logger::logInfo("Item created successfully: id=" + std::to_string(id), "ItemService");
}

Item ItemService::getItem(int id) {
    Logger::logDebug("getItem called: id=" + std::to_string(id), "ItemService");

    // OPTIMIZED: Check LRU cache first (O(1) cache hit)
    auto cached = itemCache.get(id);
    if (cached.has_value()) {
        Logger::logDebug("Cache HIT for item id=" + std::to_string(id), "ItemService");
        return cached.value();
    }

    // OPTIMIZED: O(1) HashMap lookup (was O(n) linear scan)
    auto indexIt = MockDatabase::itemIndex.find(id);
    if (indexIt != MockDatabase::itemIndex.end()) {
        Item& found = MockDatabase::items[indexIt->second];
        Logger::logDebug("Item found via index: " + found.name, "ItemService");
        // Populate cache for future accesses
        itemCache.put(id, found);
        return found;
    }

    Logger::logWarn("Item not found: id=" + std::to_string(id), "ItemService");
    throw ItemNotFoundException(id);
}

void ItemService::updateItem(int id, std::string name, int sellIn, int quality) {
    Logger::logDebug("updateItem called: id=" + std::to_string(id), "ItemService");

    // OPTIMIZED: O(1) lookup using HashMap index (was O(n) linear scan)
    auto indexIt = MockDatabase::itemIndex.find(id);
    if (indexIt != MockDatabase::itemIndex.end()) {
        Item& item = MockDatabase::items[indexIt->second];
        item.name = name;
        item.sellIn = sellIn;
        item.quality = quality;
        Logger::logInfo("Item updated: id=" + std::to_string(id) + " -> name=" + name, "ItemService");

        // OPTIMIZED: Invalidate stale cache entry, then cache updated item
        itemCache.invalidate(id);
        itemCache.put(id, item);
        return;
    }

    Logger::logWarn("Update failed - item not found: id=" + std::to_string(id), "ItemService");
    throw ItemNotFoundException(id);
}

void ItemService::deleteItem(int id) {
    Logger::logDebug("deleteItem called: id=" + std::to_string(id), "ItemService");

    // OPTIMIZED: O(1) lookup using HashMap index (was O(n) linear scan)
    auto indexIt = MockDatabase::itemIndex.find(id);
    if (indexIt != MockDatabase::itemIndex.end()) {
        size_t pos = indexIt->second;
        Logger::logInfo("Item deleted: id=" + std::to_string(id) + ", name=" + MockDatabase::items[pos].name, "ItemService");
        MockDatabase::items.erase(MockDatabase::items.begin() + pos);
        MockDatabase::rebuildIndex(); // Rebuild index after structural change

        // OPTIMIZED: Invalidate cache entry for deleted item
        itemCache.invalidate(id);
        return;
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
