#ifndef ITEM_SERVICE_H
#define ITEM_SERVICE_H

#include "../model/Item.h"
#include "../utils/LRUCache.h"
#include <vector>
#include <string>

/**
 * @class ItemService
 * @brief Handles core business logic for managing items.
 * 
 * Provides CRUD operations and business specific queries (like warning checks).
 * 
 * Performance optimizations (Experiment 3):
 * - Uses MockDatabase::itemIndex (HashMap) for O(1) lookups instead of O(n) scans
 * - Integrates LRU Cache for frequently accessed items
 */
class ItemService {
public:
    /**
     * @brief Creates a new item in the inventory.
     * @param id The unique identifier.
     * @param name The name of the item.
     * @param sellIn The days left to sell.
     * @param quality The quality value (0-50).
     * @throws ValidationException if parameters are invalid.
     * @throws ValidationException if ID already exists.
     */
    void createItem(int id, std::string name, int sellIn, int quality);

    /**
     * @brief Retrieves an item by its ID.
     * 
     * Uses LRU cache for O(1) cached lookups, falls back to
     * HashMap index for O(1) uncached lookups.
     * 
     * @param id The item identifier.
     * @return The found Item object.
     * @throws ItemNotFoundException if the item does not exist.
     */
    Item getItem(int id);

    /**
     * @brief Updates an existing item.
     * @param id The item identifier.
     * @param name The new name.
     * @param sellIn The new sellIn value.
     * @param quality The new quality value.
     * @throws ItemNotFoundException if the item does not exist.
     * @throws ValidationException if quality bounds are exceeded.
     */
    void updateItem(int id, std::string name, int sellIn, int quality);

    /**
     * @brief Deletes an item from the inventory.
     * @param id The item identifier.
     */
    void deleteItem(int id);

    /**
     * @brief Checks the inventory for items that need attention.
     * @return A list of items that are close to expiring or have low quality.
     */
    std::vector<Item> checkInventoryWarning();

    /**
     * @brief Retrieves all items.
     * @return A list of all items in the inventory.
     */
    std::vector<Item> getAllItems();

private:
    /// LRU Cache: caches recently accessed items for O(1) repeated lookups
    LRUCache<int, Item> itemCache{64};
};

#endif // ITEM_SERVICE_H
