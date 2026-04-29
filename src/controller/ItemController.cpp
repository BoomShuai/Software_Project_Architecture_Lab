#include "ItemController.h"
#include "../utils/Logger.h"
#include <iostream>

void ItemController::handleAddItemRequest(std::string token, int id, std::string name, int sellIn, int quality) {
    if (!tokenInterceptor.preHandle(token)) {
        Logger::logError("Unauthorized");
        return;
    }
    
    try {
        itemService.createItem(id, name, sellIn, quality);
        Logger::logInfo("Item created successfully.");
    } catch (std::exception& e) {
        Logger::logError("Failed to add item.");
    }
}

void ItemController::handleGetItemRequest(std::string token, int id) {
    if (!tokenInterceptor.preHandle(token)) {
        Logger::logError("Unauthorized");
        return;
    }
    
    try {
        Item item = itemService.getItem(id);
        std::cout << "Item found: " << item.name << " " << item.sellIn << " " << item.quality << std::endl;
        Logger::logInfo("Item retrieved.");
    } catch (std::exception& e) {
        Logger::logError("Item not found!");
    }
}

void ItemController::handleUpdateItemRequest(std::string token, int id, std::string name, int sellIn, int quality) {
    if (!tokenInterceptor.preHandle(token)) {
        Logger::logError("Unauthorized");
        return;
    }
    
    try {
        itemService.updateItem(id, name, sellIn, quality);
        Logger::logInfo("Item updated.");
    } catch (std::exception& e) {
        Logger::logError("Error updating item.");
    }
}

void ItemController::handleDeleteItemRequest(std::string token, int id) {
    if (!tokenInterceptor.preHandle(token)) {
        Logger::logError("Unauthorized");
        return;
    }
    
    itemService.deleteItem(id);
    Logger::logInfo("Item deleted if it existed.");
}

void ItemController::handleDailySettlementRequest(std::string token) {
    if (!tokenInterceptor.preHandle(token)) {
        Logger::logError("Unauthorized");
        return;
    }
    
    gildedRoseService.dailySettlement();
    Logger::logInfo("Daily settlement finished.");
}

void ItemController::handleInventoryWarningRequest(std::string token) {
    if (!tokenInterceptor.preHandle(token)) {
        Logger::logError("Unauthorized");
        return;
    }
    
    std::vector<Item> warnings = itemService.checkInventoryWarning();
    std::cout << "--- Warning List ---" << std::endl;
    for (int i = 0; i < warnings.size(); i++) {
        std::cout << "Warning for item: " << warnings[i].name << " (id: " << warnings[i].id << ")" << std::endl;
    }
    Logger::logInfo("Warning list generated.");
}
