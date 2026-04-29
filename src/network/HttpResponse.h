#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <map>

class HttpResponse {
private:
    int statusCode;
    std::string body;
    std::map<std::string, std::string> headers;

public:
    HttpResponse();
    
    void setStatusCode(int code);
    int getStatusCode();
    
    void setBody(std::string b);
    std::string getBody();
    
    void setHeader(std::string key, std::string value);
    
    std::string generateRawResponse();
};

#endif // HTTP_RESPONSE_H
