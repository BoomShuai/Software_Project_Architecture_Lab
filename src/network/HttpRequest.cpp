#include "HttpRequest.h"
#include <iostream>
#include <cstring>
#include <cctype>

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

void HttpRequest::parseRawRequest(std::string raw) {
    if (raw.empty()) return;

    // --- Parse request line: METHOD SP target SP HTTP/x.y ---
    size_t firstSpace = raw.find(' ');
    if (firstSpace != std::string::npos) {
        method = raw.substr(0, firstSpace);
        size_t secondSpace = raw.find(' ', firstSpace + 1);
        if (secondSpace != std::string::npos) {
            std::string target = raw.substr(firstSpace + 1, secondSpace - firstSpace - 1);

            // Split the target into path and query string. A real socket server
            // hands us "/api/items?id=9", not pre-parsed params, so the server
            // layer must do its own parsing to support scale-out clients (k6).
            size_t qmark = target.find('?');
            if (qmark != std::string::npos) {
                path = target.substr(0, qmark);
                parseQueryString(target.substr(qmark + 1));
            } else {
                path = target;
            }
        }
    }

    // --- Parse headers and body (only present in a full raw HTTP message) ---
    size_t lineEnd = raw.find("\r\n");
    if (lineEnd == std::string::npos) return; // request-line only (legacy callers)

    size_t pos = lineEnd + 2;
    size_t headerEnd = raw.find("\r\n\r\n", pos);
    std::string headerBlock = (headerEnd == std::string::npos)
                                  ? raw.substr(pos)
                                  : raw.substr(pos, headerEnd - pos);

    size_t start = 0;
    while (start < headerBlock.size()) {
        size_t end = headerBlock.find("\r\n", start);
        std::string line = (end == std::string::npos)
                               ? headerBlock.substr(start)
                               : headerBlock.substr(start, end - start);
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            size_t vstart = val.find_first_not_of(' '); // trim leading space
            if (vstart != std::string::npos) val = val.substr(vstart);
            headers[key] = val;
        }
        if (end == std::string::npos) break;
        start = end + 2;
    }

    if (headerEnd != std::string::npos) {
        body = raw.substr(headerEnd + 4);
    }
}

void HttpRequest::parseQueryString(const std::string& query) {
    size_t start = 0;
    while (start < query.size()) {
        size_t amp = query.find('&', start);
        std::string pair = (amp == std::string::npos)
                               ? query.substr(start)
                               : query.substr(start, amp - start);
        size_t eq = pair.find('=');
        if (eq != std::string::npos) {
            queryParams[urlDecode(pair.substr(0, eq))] =
                urlDecode(pair.substr(eq + 1));
        }
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
}

std::string HttpRequest::urlDecode(const std::string& s) {
    auto hexVal = [](char c) -> int {
        if (std::isdigit((unsigned char)c)) return c - '0';
        if (std::isxdigit((unsigned char)c)) return std::tolower(c) - 'a' + 10;
        return -1;
    };
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            out += ' ';
        } else if (s[i] == '%' && i + 2 < s.size()) {
            int hi = hexVal(s[i + 1]);
            int lo = hexVal(s[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out += static_cast<char>(hi * 16 + lo);
                i += 2;
            } else {
                out += s[i];
            }
        } else {
            out += s[i];
        }
    }
    return out;
}
