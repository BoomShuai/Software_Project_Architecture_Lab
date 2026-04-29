#include "TokenInterceptor.h"
#include "../utils/Constants.h"
#include <iostream>

bool TokenInterceptor::preHandle(std::string token) {
    if (token == "") {
        return false;
    }
    
    // magic numbers and logic smells removed
    if (token.length() < Constants::MIN_TOKEN_LEN) {
        return false;
    }
    
    if (token.substr(0, Constants::TOKEN_PREFIX.length()) != Constants::TOKEN_PREFIX) {
        return false;
    }
    
    std::cout << "Token interceptor pass." << std::endl;
    return true;
}
