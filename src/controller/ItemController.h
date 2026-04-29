#ifndef ITEM_CONTROLLER_H
#define ITEM_CONTROLLER_H

#include "../service/ItemService.h"
#include "../service/GildedRoseService.h"
#include "../interceptor/TokenInterceptor.h"
#include <string>

class ItemController {
private:
    ItemService itemService;
    GildedRoseService gildedRoseService;
    TokenInterceptor tokenInterceptor;

public:
    void handleAddItemRequest(std::string token, int id, std::string name, int sellIn, int quality);
    void handleGetItemRequest(std::string token, int id);
    void handleUpdateItemRequest(std::string token, int id, std::string name, int sellIn, int quality);
    void handleDeleteItemRequest(std::string token, int id);
    void handleDailySettlementRequest(std::string token);
    void handleInventoryWarningRequest(std::string token);
};

#endif // ITEM_CONTROLLER_H
