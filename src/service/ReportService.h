#ifndef REPORT_SERVICE_H
#define REPORT_SERVICE_H

#include <string>

class ReportService {
public:
    std::string generateDailyInventoryReport();
    std::string generateQualityAnalysisReport();
    void sendReportByEmail(std::string reportData, std::string toEmail);
};

#endif // REPORT_SERVICE_H
