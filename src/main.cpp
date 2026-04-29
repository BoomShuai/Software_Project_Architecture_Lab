#include "repository/MockDatabase.h"
#include "controller/AuthController.h"
#include "controller/ItemController.h"
#include "controller/Dispatcher.h"
#include "service/ReportService.h"
#include "service/ExportService.h"
#include "utils/Logger.h"
#include "network/HttpRequest.h"
#include <iostream>

int main() {
    Logger::logInfo("Starting GildedRose Backend Application V2...");
    
    // Initialize mock database
    MockDatabase::init();
    
    AuthController authController;
    ItemController itemController;
    Dispatcher dispatcher(&itemController, &authController);
    
    // Simulate Login HTTP Request
    HttpRequest loginReq("POST", "/api/auth/login");
    loginReq.setQueryParam("username", "admin");
    loginReq.setQueryParam("password", "123456");
    HttpResponse loginRes = dispatcher.dispatch(loginReq);
    
    std::string token = "";
    if (loginRes.getStatusCode() == 200) {
        token = "Bearer admin_secret_token"; // Mock extracted token
    } else {
        Logger::logError("Startup failed due to auth error.");
        return -1;
    }
    
    Logger::logInfo("Running tests with token: " + token);
    
    // Simulate Add Item Request
    HttpRequest addReq("POST", "/api/items");
    addReq.setHeader("Authorization", token);
    addReq.setQueryParam("id", "9");
    addReq.setQueryParam("name", "New Normal Item");
    addReq.setQueryParam("sellIn", "10");
    addReq.setQueryParam("quality", "20");
    dispatcher.dispatch(addReq);
    
    // Simulate Settlement
    HttpRequest settleReq("POST", "/api/admin/settle");
    settleReq.setHeader("Authorization", token);
    dispatcher.dispatch(settleReq);
    
    // Test Report & Export (New features)
    ReportService reportService;
    std::string dailyReport = reportService.generateDailyInventoryReport();
    std::cout << dailyReport << std::endl;
    
    ExportService exportService;
    std::string csvExport = exportService.exportAllItemsToCSV();
    Logger::logInfo("Exported CSV Length: " + std::to_string(csvExport.length()));
    
    Logger::logInfo("Application execution completed successfully.");
    return 0;
}
