#include "TokenInterceptor.h"
#include <iostream>

bool TokenInterceptor::preHandle(std::string token) {
    if (token == "") {
        return false;
    }
    
    // magic numbers and logic smells
    if (token.length() < 10) {
        return false;
    }
    
    if (token.substr(0, 6) != "Bearer") {
        return false;
    }
    
    std::cout << "Token interceptor pass." << std::endl;
    return true;
}
