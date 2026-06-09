#ifndef MOCK_DATABASE_H
#define MOCK_DATABASE_H

#include "../model/Item.h"
#include "../model/User.h"
#include <vector>
#include <unordered_map>
#include <sqlite3.h>

/**
 * @class MockDatabase
 * @brief In-memory database with O(1) hash-map index for item lookups.
 *
 * Performance optimization: Added itemIndex (unordered_map) to provide
 * O(1) lookups by item ID, replacing the original O(n) linear scans.
 * The index maps item ID -> position in the items vector.
 */
class MockDatabase {
public:
    static std::vector<Item> items;
    static std::vector<User> users;
    static sqlite3* db;

    /// Hash-map index: item ID -> index in items vector (O(1) lookup)
    static std::unordered_map<int, size_t> itemIndex;
    
    static void init();
    static void createRealDatabase();
    static std::string safeQuery(std::string userInput);

    /**
     * @brief Seeds @p count synthetic items to simulate a large inventory.
     *
     * Used by the scalability experiment to make read-heavy endpoints do
     * real O(n) work. Not invoked by unit tests (they keep the 8-item set).
     */
    static void seedLargeInventory(size_t count);

    /**
     * @brief Rebuilds the item ID -> vector index mapping.
     *
     * Must be called after any structural modification to the items vector
     * (add, delete, or bulk operations). Cost: O(n) one-time rebuild.
     */
    static void rebuildIndex();
};

#endif // MOCK_DATABASE_H
