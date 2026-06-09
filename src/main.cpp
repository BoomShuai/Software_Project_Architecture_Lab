#include "repository/MockDatabase.h"
#include "controller/AuthController.h"
#include "controller/ItemController.h"
#include "controller/Dispatcher.h"
#include "service/ReportService.h"
#include "service/ExportService.h"
#include "utils/Logger.h"
#include "network/HttpRequest.h"
#include "network/HttpServer.h"

#include <csignal>
#include <iostream>
#include <string>

namespace {
HttpServer* g_server = nullptr;

void handleSignal(int) {
    if (g_server) g_server->stop();
}

// Experiment 4: run the backend as a real, network-exposed HTTP server so it
// can be driven by external load-testing tools (k6/ab). Endpoints reuse the
// existing Dispatcher routing untouched.
//
// SECURITY NOTE: this server binds 0.0.0.0 with no TLS and only the existing
// mock-token auth on protected routes; it is intended for local benchmarking,
// not production exposure.
int runServer(int port, HttpServer::Mode mode, size_t threads) {
    Logger::logInfo("Starting GildedRose Backend (HTTP server mode)...");
    MockDatabase::init();

    static AuthController authController;
    static ItemController itemController;
    static Dispatcher dispatcher(&itemController, &authController);

    HttpServer server(&dispatcher, port, mode, threads);
    g_server = &server;
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    server.run();
    Logger::logInfo("Server stopped.");
    return 0;
}

// Original in-process simulation (kept for backward compatibility / demos).
int runSimulation() {
    Logger::logInfo("Starting GildedRose Backend Application V2...");
    MockDatabase::init();

    AuthController authController;
    ItemController itemController;
    Dispatcher dispatcher(&itemController, &authController);

    HttpRequest loginReq("POST", "/api/auth/login");
    loginReq.setQueryParam("username", "admin");
    loginReq.setQueryParam("password", "123456");
    HttpResponse loginRes = dispatcher.dispatch(loginReq);

    std::string token = "";
    if (loginRes.getStatusCode() == 200) {
        token = "Bearer admin_secret_token";
    } else {
        Logger::logError("Startup failed due to auth error.");
        return -1;
    }

    Logger::logInfo("Running tests with token: " + token);

    HttpRequest addReq("POST", "/api/items");
    addReq.setHeader("Authorization", token);
    addReq.setQueryParam("id", "9");
    addReq.setQueryParam("name", "New Normal Item");
    addReq.setQueryParam("sellIn", "10");
    addReq.setQueryParam("quality", "20");
    dispatcher.dispatch(addReq);

    HttpRequest settleReq("POST", "/api/admin/settle");
    settleReq.setHeader("Authorization", token);
    dispatcher.dispatch(settleReq);

    ReportService reportService;
    std::string dailyReport = reportService.generateDailyInventoryReport();
    std::cout << dailyReport << std::endl;

    ExportService exportService;
    std::string csvExport = exportService.exportAllItemsToCSV();
    Logger::logInfo("Exported CSV Length: " + std::to_string(csvExport.length()));

    Logger::logInfo("Application execution completed successfully.");
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    bool serve = false;
    int port = 8080;
    HttpServer::Mode mode = HttpServer::Mode::THREAD_POOL;
    size_t threads = 8;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--serve") {
            serve = true;
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--mode" && i + 1 < argc) {
            std::string m = argv[++i];
            mode = (m == "single") ? HttpServer::Mode::SINGLE_THREAD
                                   : HttpServer::Mode::THREAD_POOL;
        } else if (arg == "--threads" && i + 1 < argc) {
            threads = static_cast<size_t>(std::stoul(argv[++i]));
        }
    }

    // Keep the server quiet under load so logging I/O does not dominate the
    // benchmark; errors are still surfaced.
    if (serve) {
        Logger::setLevel(Logger::Level::ERROR);
        return runServer(port, mode, threads);
    }
    return runSimulation();
}
