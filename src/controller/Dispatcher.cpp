#include "Dispatcher.h"
#include "../utils/Logger.h"
#include "../utils/StringUtil.h"
#include "../repository/MockDatabase.h"
#include "../interceptor/TokenInterceptor.h"
#include "../utils/Exceptions.h"
#include "../service/ReportService.h"
#include "../service/CacheService.h"
#include <stdexcept>
#include <exception>
#include <cstdlib>

Dispatcher::Dispatcher(ItemController* ic, AuthController* ac) {
    itemController = ic;
    authController = ac;
    // Caching is opt-in via env so the before/after load test can toggle it
    // without recompiling: GR_CACHE=1 enables the report cache.
    cacheEnabled_ = (std::getenv("GR_CACHE") != nullptr);
}

HttpResponse Dispatcher::dispatch(HttpRequest request) {
    HttpResponse response;
    std::string path = request.getPath();
    std::string method = request.getMethod();
    std::string token = request.getHeader("Authorization");
    
    Logger::logInfo("Dispatching request: " + method + " " + path);
    
    try {
        // AOP: Intercept all paths except public endpoints
        if (path != "/api/auth/login") {
            TokenInterceptor interceptor;
            interceptor.preHandle(token);
        }
        
        // Very sloppy routing with lots of if-else
        if (path == "/api/auth/login" && method == "POST") {
            std::string user = request.getQueryParam("username");
            std::string pass = request.getQueryParam("password");
            std::string respToken = authController->handleLoginRequest(user, pass);
            if (respToken != "") {
                response.setStatusCode(200);
                response.setBody("{\"token\":\"" + respToken + "\"}");
            } else {
                response.setStatusCode(401);
                response.setBody("{\"error\":\"Login failed\"}");
            }
        } 
        else if (path == "/api/items" && method == "GET") {
            std::string idStr = request.getQueryParam("id");
            if (idStr != "") {
                int id = std::stoi(idStr);
                itemController->handleGetItemRequest(id);
                response.setStatusCode(200);
                response.setBody("{\"status\":\"success\"}");
            } else {
                response.setStatusCode(400);
                response.setBody("{\"error\":\"Missing id\"}");
            }
        }
        else if (path == "/api/items" && method == "POST") {
            int id = std::stoi(request.getQueryParam("id"));
            std::string name = request.getQueryParam("name");
            int sellIn = std::stoi(request.getQueryParam("sellIn"));
            int quality = std::stoi(request.getQueryParam("quality"));
            
            itemController->handleAddItemRequest(id, name, sellIn, quality);
            response.setStatusCode(201);
            response.setBody("{\"status\":\"created\"}");
        }
        else if (path == "/api/items" && method == "PUT") {
            int id = std::stoi(request.getQueryParam("id"));
            std::string name = request.getQueryParam("name");
            int sellIn = std::stoi(request.getQueryParam("sellIn"));
            int quality = std::stoi(request.getQueryParam("quality"));
            
            itemController->handleUpdateItemRequest(id, name, sellIn, quality);
            response.setStatusCode(200);
            response.setBody("{\"status\":\"updated\"}");
        }
        else if (path == "/api/items" && method == "DELETE") {
            int id = std::stoi(request.getQueryParam("id"));
            itemController->handleDeleteItemRequest(id);
            response.setStatusCode(200);
            response.setBody("{\"status\":\"deleted\"}");
        }
        else if (path == "/api/admin/settle" && method == "POST") {
            itemController->handleDailySettlementRequest();
            response.setStatusCode(200);
            response.setBody("{\"status\":\"settled\"}");
        }
        else if (path == "/api/admin/warnings" && method == "GET") {
            itemController->handleInventoryWarningRequest();
            response.setStatusCode(200);
            response.setBody("{\"status\":\"warnings checked\"}");
        }
        // SAFE ENDPOINT
        else if (path == "/api/admin/search" && method == "GET") {
            std::string rawInput = request.getQueryParam("username");
            std::string dbResult = MockDatabase::safeQuery(rawInput);
            response.setStatusCode(200);
            response.setBody(dbResult);
        }
        // READ-HEAVY ENDPOINT (scalability hot path).
        // Recomputes an O(n) aggregate over the whole inventory. Under load
        // this is the CPU bottleneck the cache + thread-pool target.
        else if (path == "/api/reports/daily" && method == "GET") {
            const std::string cacheKey = "report:daily";
            if (cacheEnabled_) {
                auto cached = cache_.get(cacheKey);
                if (cached.has_value()) {
                    response.setStatusCode(200);
                    response.setHeader("X-Cache", "HIT");
                    response.setBody(*cached);
                    return response;
                }
            }
            ReportService reportService;
            std::string report = reportService.generateDailyInventoryReport();
            std::string jsonBody =
                "{\"report\":\"" + StringUtil::escapeJson(report) + "\"}";
            if (cacheEnabled_) {
                cache_.set(cacheKey, jsonBody, 30);
            }
            response.setStatusCode(200);
            response.setHeader("X-Cache", cacheEnabled_ ? "MISS" : "OFF");
            response.setBody(jsonBody);
        }
        else {
            response.setStatusCode(404);
            response.setBody("{\"error\":\"Not found\"}");
        }
    }
    catch (const UnauthorizedException& e) {
        response.setStatusCode(401);
        response.setBody("{\"error\":\"Unauthorized: Invalid Token\"}");
    }
    catch (const std::invalid_argument& e) {
        response.setStatusCode(400);
        response.setBody("{\"error\":\"Invalid parameter format\"}");
    }
    catch (const std::exception& e) {
        // Catch custom and generic exceptions
        response.setStatusCode(500);
        response.setBody(std::string("{\"error\":\"") + e.what() + "\"}");
    }
    
    return response;
}
