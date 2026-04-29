#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include "../service/AuthService.h"
#include <string>

class AuthController {
private:
    AuthService authService;

public:
    std::string handleLoginRequest(std::string username, std::string password);
};

#endif // AUTH_CONTROLLER_H
