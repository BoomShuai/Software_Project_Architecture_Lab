#include <gtest/gtest.h>
#include "service/ReportService.h"
#include "repository/MockDatabase.h"

class ReportServiceTest : public ::testing::Test {
protected:
    ReportService service;
    void SetUp() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
    void TearDown() override {
        MockDatabase::items.clear();
        MockDatabase::users.clear();
    }
};

// === Daily Inventory Report ===

TEST_F(ReportServiceTest, DailyReport_EmptyInventory) {
    std::string report = service.generateDailyInventoryReport();
    EXPECT_NE(report.find("DAILY INVENTORY REPORT"), std::string::npos);
    EXPECT_NE(report.find("Total Items: 0"), std::string::npos);
    EXPECT_NE(report.find("Items Expired: 0"), std::string::npos);
    EXPECT_NE(report.find("Items Expiring Soon: 0"), std::string::npos);
}

TEST_F(ReportServiceTest, DailyReport_WithExpiredItems) {
    MockDatabase::items.push_back(Item(1, "Expired", -1, 5));
    MockDatabase::items.push_back(Item(2, "AlsoExpired", -5, 0));
    std::string report = service.generateDailyInventoryReport();
    EXPECT_NE(report.find("Items Expired: 2"), std::string::npos);
}

TEST_F(ReportServiceTest, DailyReport_WithExpiringSoonItems) {
    MockDatabase::items.push_back(Item(1, "AlmostGone", 2, 10));
    MockDatabase::items.push_back(Item(2, "SoonExpiring", 4, 20));
    std::string report = service.generateDailyInventoryReport();
    EXPECT_NE(report.find("Items Expiring Soon: 2"), std::string::npos);
}

TEST_F(ReportServiceTest, DailyReport_MixedItems) {
    MockDatabase::items.push_back(Item(1, "Normal", 10, 20));     // normal
    MockDatabase::items.push_back(Item(2, "Expiring", 3, 10));    // expiring soon
    MockDatabase::items.push_back(Item(3, "Expired", -1, 0));     // expired
    std::string report = service.generateDailyInventoryReport();
    EXPECT_NE(report.find("Total Items: 3"), std::string::npos);
    EXPECT_NE(report.find("Items Expired: 1"), std::string::npos);
    EXPECT_NE(report.find("Items Expiring Soon: 1"), std::string::npos);
}

TEST_F(ReportServiceTest, DailyReport_BoundarySellIn0_NotExpiring) {
    // sellIn == 0 is not < 0 and not < 5, so it's "expiring soon"
    MockDatabase::items.push_back(Item(1, "Edge", 0, 10));
    std::string report = service.generateDailyInventoryReport();
    EXPECT_NE(report.find("Items Expiring Soon: 1"), std::string::npos);
    EXPECT_NE(report.find("Items Expired: 0"), std::string::npos);
}

// === Quality Analysis Report ===

TEST_F(ReportServiceTest, QualityReport_EmptyInventory) {
    std::string report = service.generateQualityAnalysisReport();
    EXPECT_NE(report.find("QUALITY ANALYSIS REPORT"), std::string::npos);
    EXPECT_NE(report.find("High Quality Items"), std::string::npos);
}

TEST_F(ReportServiceTest, QualityReport_HighQualityItems) {
    MockDatabase::items.push_back(Item(1, "Premium", 10, 50));
    MockDatabase::items.push_back(Item(2, "Gold", 10, 35));
    std::string report = service.generateQualityAnalysisReport();
    EXPECT_NE(report.find("High Quality Items (>30): 2"), std::string::npos);
}

TEST_F(ReportServiceTest, QualityReport_MediumQualityItems) {
    MockDatabase::items.push_back(Item(1, "Silver", 10, 20));
    MockDatabase::items.push_back(Item(2, "Bronze", 10, 15));
    std::string report = service.generateQualityAnalysisReport();
    EXPECT_NE(report.find("Medium Quality Items (11-30): 2"), std::string::npos);
}

TEST_F(ReportServiceTest, QualityReport_LowQualityItems) {
    MockDatabase::items.push_back(Item(1, "Junk", 10, 5));
    MockDatabase::items.push_back(Item(2, "Trash", 10, 0));
    std::string report = service.generateQualityAnalysisReport();
    EXPECT_NE(report.find("Low Quality Items (<=10): 2"), std::string::npos);
}

TEST_F(ReportServiceTest, QualityReport_MixedQualities) {
    MockDatabase::items.push_back(Item(1, "High", 10, 40));
    MockDatabase::items.push_back(Item(2, "Mid", 10, 20));
    MockDatabase::items.push_back(Item(3, "Low", 10, 5));
    std::string report = service.generateQualityAnalysisReport();
    EXPECT_NE(report.find("High Quality Items (>30): 1"), std::string::npos);
    EXPECT_NE(report.find("Medium Quality Items (11-30): 1"), std::string::npos);
    EXPECT_NE(report.find("Low Quality Items (<=10): 1"), std::string::npos);
}

TEST_F(ReportServiceTest, QualityReport_BoundaryQuality10_IsLow) {
    MockDatabase::items.push_back(Item(1, "Edge10", 10, 10));
    std::string report = service.generateQualityAnalysisReport();
    EXPECT_NE(report.find("Low Quality Items (<=10): 1"), std::string::npos);
}

TEST_F(ReportServiceTest, QualityReport_BoundaryQuality11_IsMedium) {
    MockDatabase::items.push_back(Item(1, "Edge11", 10, 11));
    std::string report = service.generateQualityAnalysisReport();
    EXPECT_NE(report.find("Medium Quality Items (11-30): 1"), std::string::npos);
}

TEST_F(ReportServiceTest, QualityReport_BoundaryQuality30_IsMedium) {
    MockDatabase::items.push_back(Item(1, "Edge30", 10, 30));
    std::string report = service.generateQualityAnalysisReport();
    EXPECT_NE(report.find("Medium Quality Items (11-30): 1"), std::string::npos);
}

TEST_F(ReportServiceTest, QualityReport_BoundaryQuality31_IsHigh) {
    MockDatabase::items.push_back(Item(1, "Edge31", 10, 31));
    std::string report = service.generateQualityAnalysisReport();
    EXPECT_NE(report.find("High Quality Items (>30): 1"), std::string::npos);
}

// === Send Report ===

TEST_F(ReportServiceTest, SendReport_DoesNotThrow) {
    EXPECT_NO_THROW(service.sendReportByEmail("test data", "test@example.com"));
}
