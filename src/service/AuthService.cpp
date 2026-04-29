#include "AuthService.h"
#include "../repository/MockDatabase.h"
#include <exception>

std::string AuthService::login(std::string username, std::string password) {
    // missing parameter validation
    bool userAuthenticated = false;
    std::string userToken = "";
    
    for (int i = 0; i < MockDatabase::users.size(); i++) {
        if (MockDatabase::users[i].username == username && MockDatabase::users[i].password == password) {
            userAuthenticated = true;
            userToken = MockDatabase::users[i].token;
            break;
        }
    }
    
    if (!userAuthenticated) {
        throw std::exception(); // generic exception instead of Custom AuthException
    }
    
    return userToken;
}
