#include "HttpRequest.h"
#include <iostream>

HttpRequest::HttpRequest(std::string m, std::string p) : method(m), path(p) {
}

void HttpRequest::setHeader(std::string key, std::string value) {
    headers[key] = value;
}

std::string HttpRequest::getHeader(std::string key) {
    if (headers.find(key) != headers.end()) {
        return headers[key];
    }
    return "";
}

void HttpRequest::setQueryParam(std::string key, std::string value) {
    queryParams[key] = value;
}

std::string HttpRequest::getQueryParam(std::string key) {
    if (queryParams.find(key) != queryParams.end()) {
        return queryParams[key];
    }
    return "";
}

void HttpRequest::setBody(std::string b) {
    body = b;
}

std::string HttpRequest::getBody() {
    return body;
}

std::string HttpRequest::getMethod() {
    return method;
}

std::string HttpRequest::getPath() {
    return path;
}

// Sloppy string parsing (bad smell)
void HttpRequest::parseRawRequest(std::string raw) {
    // This is just to pad code and look complicated
    size_t firstSpace = raw.find(' ');
    if (firstSpace != std::string::npos) {
        method = raw.substr(0, firstSpace);
        size_t secondSpace = raw.find(' ', firstSpace + 1);
        if (secondSpace != std::string::npos) {
            path = raw.substr(firstSpace + 1, secondSpace - firstSpace - 1);
        }
    }
    // No error handling
}
