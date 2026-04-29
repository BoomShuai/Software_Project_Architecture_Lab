#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <string>

/**
 * @class AuthService
 * @brief Handles user authentication and token generation.
 */
class AuthService {
public:
    /**
     * @brief Authenticates a user with the given credentials.
     * @param username The user's username.
     * @param password The user's password.
     * @return A valid authorization token if login succeeds.
     * @throws ValidationException if username or password is too short.
     * @throws AuthenticationException if credentials do not match.
     */
    std::string login(std::string username, std::string password);
};

#endif // AUTH_SERVICE_H
