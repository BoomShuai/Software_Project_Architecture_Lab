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
    // Map status codes to their HTTP reason phrases so real clients (k6, curl)
    // see correct status lines instead of a hardcoded "OK".
    std::string reason;
    switch (statusCode) {
        case 200: reason = "OK"; break;
        case 201: reason = "Created"; break;
        case 400: reason = "Bad Request"; break;
        case 401: reason = "Unauthorized"; break;
        case 404: reason = "Not Found"; break;
        case 500: reason = "Internal Server Error"; break;
        default:  reason = "OK"; break;
    }

    std::string raw = "HTTP/1.1 " + std::to_string(statusCode) + " " + reason + "\r\n";
    for (auto const& [key, val] : headers) {
        raw += key + ": " + val + "\r\n";
    }
    // Content-Length lets clients reuse the connection (keep-alive), which is
    // essential for sustaining high request rates during load testing.
    raw += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    raw += "\r\n" + body;
    return raw;
}
