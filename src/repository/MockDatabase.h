#ifndef MOCK_DATABASE_H
#define MOCK_DATABASE_H

#include "../model/Item.h"
#include "../model/User.h"
#include <vector>
#include <sqlite3.h>

class MockDatabase {
public:
    static std::vector<Item> items;
    static std::vector<User> users;
    static sqlite3* db;
    
    static void init();
    static void createRealDatabase();
    static std::string unsafeQuery(std::string userInput);
};

#endif // MOCK_DATABASE_H
