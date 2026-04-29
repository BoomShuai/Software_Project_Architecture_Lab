#ifndef ITEM_CONTROLLER_H
#define ITEM_CONTROLLER_H

#include "../service/ItemService.h"
#include "../service/GildedRoseService.h"
#include <string>

class ItemController {
private:
    ItemService itemService;
    GildedRoseService gildedRoseService;

public:
    void handleAddItemRequest(int id, std::string name, int sellIn, int quality);
    void handleGetItemRequest(int id);
    void handleUpdateItemRequest(int id, std::string name, int sellIn, int quality);
    void handleDeleteItemRequest(int id);
    void handleDailySettlementRequest();
    void handleInventoryWarningRequest();
};

#endif // ITEM_CONTROLLER_H
