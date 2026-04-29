#include "AuthController.h"
#include "../utils/Logger.h"
#include <iostream>

std::string AuthController::handleLoginRequest(std::string username, std::string password) {
    std::string token = "";
    try {
        token = authService.login(username, password);
        Logger::logInfo("Login successful for user: " + username);
    } catch (std::exception& e) {
        Logger::logError("Login failed!");
    }
    return token;
}
