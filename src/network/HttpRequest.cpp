#include "HttpRequest.h"
#include <iostream>
#include <cstring>

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

// Sloppy string parsing with BUFFER OVERFLOW VULNERABILITY (CWE-120) for CodeQL
void HttpRequest::parseRawRequest(std::string raw) {
    char unsafeBuffer[50];
    
    // CodeQL catches strcpy with unconstrained size
    strcpy(unsafeBuffer, raw.c_str()); 
    
    // Continue parsing from the buffer
    std::string bufferedRaw(unsafeBuffer);
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
