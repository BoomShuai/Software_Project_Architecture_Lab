#include "AuthService.h"
#include "../repository/MockDatabase.h"
#include "../utils/Exceptions.h"
#include "../utils/Logger.h"
#include <cassert>

std::string AuthService::login(std::string username, std::string password) {
    Logger::logDebug("Login attempt: username=" + username, "AuthService");

    // Pre-condition assertion: inputs must not be empty
    if (username.empty() || password.empty()) {
        Logger::logWarn("Login rejected: empty credentials", "AuthService");
        throw ValidationException("Username and password must not be empty");
    }

    bool userAuthenticated = false;
    std::string userToken = "";

    for (size_t i = 0; i < MockDatabase::users.size(); i++) {
        if (MockDatabase::users[i].username == username && MockDatabase::users[i].password == password) {
            userAuthenticated = true;
            userToken = MockDatabase::users[i].token;
            break;
        }
    }

    if (!userAuthenticated) {
        Logger::logError("Authentication failed for user: " + username, "AuthService");
        throw AuthenticationException("Invalid username or password");
    }

    // Post-condition: token must not be empty on success
    assert(!userToken.empty() && "Post-condition failed: token should not be empty after successful auth");
    Logger::logInfo("User authenticated successfully: " + username, "AuthService");
    return userToken;
}
