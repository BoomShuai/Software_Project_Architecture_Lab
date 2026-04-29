#ifndef MOCK_DATABASE_H
#define MOCK_DATABASE_H

#include "../model/Item.h"
#include "../model/User.h"
#include <vector>

class MockDatabase {
public:
    static std::vector<Item> items;
    static std::vector<User> users;
    
    static void init();
};

#endif // MOCK_DATABASE_H
