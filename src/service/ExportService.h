#ifndef EXPORT_SERVICE_H
#define EXPORT_SERVICE_H

#include <string>

class ExportService {
public:
    std::string exportAllItemsToCSV();
    std::string exportAllItemsToXML();
    std::string exportUsersToCSV();
};

#endif // EXPORT_SERVICE_H
