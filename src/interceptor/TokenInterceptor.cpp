#include "TokenInterceptor.h"
#include "../utils/Constants.h"
#include "../utils/Exceptions.h"

bool TokenInterceptor::preHandle(std::string token) {
    if (token == "") throw UnauthorizedException();
    if (token.length() < Constants::MIN_TOKEN_LEN) throw UnauthorizedException();
    if (token.substr(0, Constants::TOKEN_PREFIX.length()) != Constants::TOKEN_PREFIX) throw UnauthorizedException();
    return true;
}
