#include "ItemController.h"
#include "../utils/Logger.h"
#include <iostream>

void ItemController::handleAddItemRequest(int id, std::string name, int sellIn, int quality) {
    itemService.createItem(id, name, sellIn, quality);
    Logger::logInfo("Item created successfully.");
}

void ItemController::handleGetItemRequest(int id) {
    Item item = itemService.getItem(id);
    std::cout << "Item found: " << item.name << " " << item.sellIn << " " << item.quality << std::endl;
    Logger::logInfo("Item retrieved.");
}

void ItemController::handleUpdateItemRequest(int id, std::string name, int sellIn, int quality) {
    itemService.updateItem(id, name, sellIn, quality);
    Logger::logInfo("Item updated.");
}

void ItemController::handleDeleteItemRequest(int id) {
    itemService.deleteItem(id);
    Logger::logInfo("Item deleted if it existed.");
}

void ItemController::handleDailySettlementRequest() {
    gildedRoseService.dailySettlement();
    Logger::logInfo("Daily settlement finished.");
}

void ItemController::handleInventoryWarningRequest() {
    std::vector<Item> warnings = itemService.checkInventoryWarning();
    std::cout << "--- Warning List ---" << std::endl;
    for (int i = 0; i < warnings.size(); i++) {
        std::cout << "Warning for item: " << warnings[i].name << " (id: " << warnings[i].id << ")" << std::endl;
    }
    Logger::logInfo("Warning list generated.");
}
