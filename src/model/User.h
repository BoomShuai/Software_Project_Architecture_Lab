#ifndef USER_H
#define USER_H

#include <string>

struct User {
    int userId;
    std::string username;
    std::string password;
    std::string token;
    std::string email;
    std::string phone;
    std::string address;
    int role;
    bool isActive;
    long long lastLoginTime;

    User() : userId(0), username(""), password(""), token(""), email(""), phone(""), address(""), role(0), isActive(false), lastLoginTime(0) {}
};

#endif // USER_H
