#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <map>

class HttpRequest {
private:
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> queryParams;

public:
    HttpRequest(std::string m, std::string p);
    
    void setHeader(std::string key, std::string value);
    std::string getHeader(std::string key);
    
    void setQueryParam(std::string key, std::string value);
    std::string getQueryParam(std::string key);
    
    void setBody(std::string b);
    std::string getBody();
    
    std::string getMethod();
    std::string getPath();
    
    void parseRawRequest(std::string raw);
};

#endif // HTTP_REQUEST_H
