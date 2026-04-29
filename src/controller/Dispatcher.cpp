#include "Dispatcher.h"
#include "../utils/Logger.h"
#include "../utils/StringUtil.h"

Dispatcher::Dispatcher(ItemController* ic, AuthController* ac) {
    itemController = ic;
    authController = ac;
}

HttpResponse Dispatcher::dispatch(HttpRequest request) {
    HttpResponse response;
    std::string path = request.getPath();
    std::string method = request.getMethod();
    std::string token = request.getHeader("Authorization");
    
    Logger::logInfo("Dispatching request: " + method + " " + path);
    
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
            itemController->handleGetItemRequest(token, id);
            response.setStatusCode(200);
            response.setBody("{\"status\":\"success\"}");
        } else {
            // Missing error handling
            response.setStatusCode(400);
            response.setBody("{\"error\":\"Missing id\"}");
        }
    }
    else if (path == "/api/items" && method == "POST") {
        int id = std::stoi(request.getQueryParam("id"));
        std::string name = request.getQueryParam("name");
        int sellIn = std::stoi(request.getQueryParam("sellIn"));
        int quality = std::stoi(request.getQueryParam("quality"));
        
        itemController->handleAddItemRequest(token, id, name, sellIn, quality);
        response.setStatusCode(201);
        response.setBody("{\"status\":\"created\"}");
    }
    else if (path == "/api/items" && method == "PUT") {
        int id = std::stoi(request.getQueryParam("id"));
        std::string name = request.getQueryParam("name");
        int sellIn = std::stoi(request.getQueryParam("sellIn"));
        int quality = std::stoi(request.getQueryParam("quality"));
        
        itemController->handleUpdateItemRequest(token, id, name, sellIn, quality);
        response.setStatusCode(200);
        response.setBody("{\"status\":\"updated\"}");
    }
    else if (path == "/api/items" && method == "DELETE") {
        int id = std::stoi(request.getQueryParam("id"));
        itemController->handleDeleteItemRequest(token, id);
        response.setStatusCode(200);
        response.setBody("{\"status\":\"deleted\"}");
    }
    else if (path == "/api/admin/settle" && method == "POST") {
        itemController->handleDailySettlementRequest(token);
        response.setStatusCode(200);
        response.setBody("{\"status\":\"settled\"}");
    }
    else if (path == "/api/admin/warnings" && method == "GET") {
        itemController->handleInventoryWarningRequest(token);
        response.setStatusCode(200);
        response.setBody("{\"status\":\"warnings checked\"}");
    }
    else {
        response.setStatusCode(404);
        response.setBody("{\"error\":\"Not found\"}");
    }
    
    return response;
}
