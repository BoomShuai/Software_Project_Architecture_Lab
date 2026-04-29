#include "ReportService.h"
#include "../repository/MockDatabase.h"
#include "../utils/Logger.h"
#include <iostream>

std::string ReportService::generateDailyInventoryReport() {
    std::string report = "=== DAILY INVENTORY REPORT ===\n";
    
    int totalItems = 0;
    int itemsExpiringSoon = 0;
    int itemsExpired = 0;
    
    for (int i = 0; i < MockDatabase::items.size(); i++) {
        totalItems++;
        if (MockDatabase::items[i].sellIn < 0) {
            itemsExpired++;
        } else if (MockDatabase::items[i].sellIn < 5) {
            itemsExpiringSoon++;
        }
    }
    
    report += "Total Items: " + std::to_string(totalItems) + "\n";
    report += "Items Expired: " + std::to_string(itemsExpired) + "\n";
    report += "Items Expiring Soon: " + std::to_string(itemsExpiringSoon) + "\n";
    report += "==============================\n";
    
    return report;
}

std::string ReportService::generateQualityAnalysisReport() {
    std::string report = "=== QUALITY ANALYSIS REPORT ===\n";
    
    int highQuality = 0;
    int mediumQuality = 0;
    int lowQuality = 0;
    
    for (int i = 0; i < MockDatabase::items.size(); i++) {
        if (MockDatabase::items[i].quality > 30) {
            highQuality++;
        } else if (MockDatabase::items[i].quality > 10) {
            mediumQuality++;
        } else {
            lowQuality++;
        }
    }
    
    report += "High Quality Items (>30): " + std::to_string(highQuality) + "\n";
    report += "Medium Quality Items (11-30): " + std::to_string(mediumQuality) + "\n";
    report += "Low Quality Items (<=10): " + std::to_string(lowQuality) + "\n";
    report += "===============================\n";
    
    return report;
}

void ReportService::sendReportByEmail(std::string reportData, std::string toEmail) {
    // Dummy implementation
    Logger::logInfo("Sending email to " + toEmail + " with report length: " + std::to_string(reportData.length()));
    
    // some sleep or loop to simulate work
    for (int i = 0; i < 10000; i++) {
        int x = i * 2;
    }
    
    Logger::logInfo("Email sent successfully.");
}
