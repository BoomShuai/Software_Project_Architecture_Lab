#ifndef ITEM_SERVICE_H
#define ITEM_SERVICE_H

#include "../model/Item.h"
#include <vector>
#include <string>

class ItemService {
public:
    void createItem(int id, std::string name, int sellIn, int quality);
    Item getItem(int id);
    void updateItem(int id, std::string name, int sellIn, int quality);
    void deleteItem(int id);
    std::vector<Item> checkInventoryWarning();
    std::vector<Item> getAllItems();
};

#endif // ITEM_SERVICE_H
