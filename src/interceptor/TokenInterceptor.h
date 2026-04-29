#ifndef TOKEN_INTERCEPTOR_H
#define TOKEN_INTERCEPTOR_H

#include <string>

class TokenInterceptor {
public:
    bool preHandle(std::string token);
};

#endif // TOKEN_INTERCEPTOR_H
