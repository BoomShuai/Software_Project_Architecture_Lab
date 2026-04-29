#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <string>

class AuthService {
public:
    std::string login(std::string username, std::string password);
};

#endif // AUTH_SERVICE_H
