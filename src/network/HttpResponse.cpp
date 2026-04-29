#include "HttpResponse.h"

HttpResponse::HttpResponse() {
    statusCode = 200;
    headers["Content-Type"] = "application/json";
}

void HttpResponse::setStatusCode(int code) {
    statusCode = code;
}

int HttpResponse::getStatusCode() {
    return statusCode;
}

void HttpResponse::setBody(std::string b) {
    body = b;
}

std::string HttpResponse::getBody() {
    return body;
}

void HttpResponse::setHeader(std::string key, std::string value) {
    headers[key] = value;
}

std::string HttpResponse::generateRawResponse() {
    std::string raw = "HTTP/1.1 " + std::to_string(statusCode) + " OK\r\n";
    for (auto const& [key, val] : headers) {
        raw += key + ": " + val + "\r\n";
    }
    raw += "\r\n" + body;
    return raw;
}
