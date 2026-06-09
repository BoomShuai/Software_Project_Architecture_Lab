#include "MockDatabase.h"
#include <iostream>
#include <cstdlib>

std::vector<Item> MockDatabase::items;
std::vector<User> MockDatabase::users;
sqlite3* MockDatabase::db = nullptr;
std::unordered_map<int, size_t> MockDatabase::itemIndex;

void MockDatabase::init() {
    items.push_back(Item(1, "+5 Dexterity Vest", 10, 20));
    items.push_back(Item(2, "Aged Brie", 2, 0));
    items.push_back(Item(3, "Elixir of the Mongoose", 5, 7));
    items.push_back(Item(4, "Sulfuras, Hand of Ragnaros", 0, 80));
    items.push_back(Item(5, "Sulfuras, Hand of Ragnaros", -1, 80));
    items.push_back(Item(6, "Backstage passes to a TAFKAL80ETC concert", 15, 20));
    items.push_back(Item(7, "Backstage passes to a TAFKAL80ETC concert", 10, 49));
    items.push_back(Item(8, "Backstage passes to a TAFKAL80ETC concert", 5, 49));

    User admin;
    admin.userId = 1;
    admin.username = "admin";
    admin.password = "123456";
    admin.token = "Bearer admin_secret_token";
    users.push_back(admin);

    // Scalability experiment: optionally seed a large inventory so that
    // read-heavy report endpoints do real O(n) work and the scale-up /
    // caching optimizations produce measurable differences under load.
    // Controlled by env var so the unit-test fixtures stay at 8 items.
    if (const char* seedEnv = std::getenv("GR_SEED_ITEMS")) {
        long n = std::strtol(seedEnv, nullptr, 10);
        if (n > 0) seedLargeInventory(static_cast<size_t>(n));
    }

    // Build hash-map index for O(1) lookups
    rebuildIndex();

    // Create real sqlite database for DBeaver testing
    createRealDatabase();
}

void MockDatabase::seedLargeInventory(size_t count) {
    items.reserve(items.size() + count);
    int nextId = 1000;
    const char* names[] = {"+5 Dexterity Vest", "Aged Brie",
                           "Elixir of the Mongoose",
                           "Backstage passes to a TAFKAL80ETC concert",
                           "Conjured Mana Cake"};
    for (size_t i = 0; i < count; ++i) {
        int sellIn = static_cast<int>(i % 30) - 5; // mix of expired/expiring
        int quality = static_cast<int>(i % 50);
        items.push_back(Item(nextId++, names[i % 5], sellIn, quality));
    }
}


void MockDatabase::rebuildIndex() {
    itemIndex.clear();
    itemIndex.reserve(items.size());
    for (size_t i = 0; i < items.size(); i++) {
        itemIndex[items[i].id] = i;
    }
}

void MockDatabase::createRealDatabase() {
    int rc = sqlite3_open("gilded_rose.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS USERS("  \
                      "ID INT PRIMARY KEY     NOT NULL," \
                      "USERNAME       TEXT    NOT NULL," \
                      "PASSWORD       TEXT    NOT NULL);";
                      
    char* zErrMsg = 0;
    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(zErrMsg);
    }
    
    // Insert dummy data
    const char* insertSql = "INSERT OR IGNORE INTO USERS (ID,USERNAME,PASSWORD) VALUES (1, 'admin', '123456');";
    sqlite3_exec(db, insertSql, 0, 0, 0);
}

// SAFE QUERY using prepared statements to prevent SQL Injection
std::string MockDatabase::safeQuery(std::string userInput) {
    if (!db) return "Database not connected";
    
    std::string sql = "SELECT * FROM USERS WHERE USERNAME = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::string("SQL error: ") + sqlite3_errmsg(db);
    }
    
    // Bind the parameter safely
    sqlite3_bind_text(stmt, 1, userInput.c_str(), -1, SQLITE_TRANSIENT);
    
    std::string resultStr = "";
    int cols = sqlite3_column_count(stmt);
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        for (int i = 0; i < cols; i++) {
            const char* colName = sqlite3_column_name(stmt, i);
            const char* val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            resultStr += colName;
            resultStr += " = ";
            resultStr += val ? val : "NULL";
            resultStr += "\n";
        }
    }
    
    sqlite3_finalize(stmt);
    return resultStr;
}
