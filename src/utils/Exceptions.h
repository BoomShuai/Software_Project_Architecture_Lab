#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

class ItemNotFoundException : public std::runtime_error {
public:
    explicit ItemNotFoundException(int id)
        : std::runtime_error("Item not found with id: " + std::to_string(id)) {}
};

class AuthenticationException : public std::runtime_error {
public:
    explicit AuthenticationException(const std::string& msg)
        : std::runtime_error("Authentication failed: " + msg) {}
};

class ValidationException : public std::runtime_error {
public:
    explicit ValidationException(const std::string& msg)
        : std::runtime_error("Validation error: " + msg) {}
};

class UnauthorizedException : public std::runtime_error {
public:
    explicit UnauthorizedException()
        : std::runtime_error("Unauthorized: Invalid or missing token") {}
};

#endif
