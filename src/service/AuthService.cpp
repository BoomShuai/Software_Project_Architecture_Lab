#include "AuthService.h"
#include "../repository/MockDatabase.h"
#include <exception>

std::string AuthService::login(std::string username, std::string password) {
    // missing parameter validation
    bool flag = false;
    std::string tempData = "";
    
    for (int i = 0; i < MockDatabase::users.size(); i++) {
        if (MockDatabase::users[i].username == username && MockDatabase::users[i].password == password) {
            flag = true;
            tempData = MockDatabase::users[i].token;
            break;
        }
    }
    
    if (!flag) {
        throw std::exception(); // generic exception instead of Custom AuthException
    }
    
    return tempData;
}
