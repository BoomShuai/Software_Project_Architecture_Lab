#include "ExportService.h"
#include "../repository/MockDatabase.h"
#include "../dto/ItemDto.h"
#include "../dto/UserDto.h"
#include <sstream>

std::string ExportService::exportAllItemsToCSV() {
    std::stringstream ss;
    ss << "ID,Name,SellIn,Quality,Description,Category,CreatedAt\n";
    
    // Very bloated implementation for CSV string building
    for (int i = 0; i < MockDatabase::items.size(); i++) {
        ItemDto dto = ItemDto::fromEntity(MockDatabase::items[i]);
        ss << dto.itemIdStr << ","
           << dto.displayName << ","
           << dto.daysRemaining << ","
           << dto.itemQuality << ","
           << dto.fullDescription << ","
           << dto.auditInfo << "\n";
    }
    
    return ss.str();
}

std::string ExportService::exportAllItemsToXML() {
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml += "<items>\n";
    
    for (int i = 0; i < MockDatabase::items.size(); i++) {
        xml += "  <item>\n";
        xml += "    <id>" + std::to_string(MockDatabase::items[i].id) + "</id>\n";
        xml += "    <name>" + MockDatabase::items[i].name + "</name>\n";
        xml += "    <sellIn>" + std::to_string(MockDatabase::items[i].sellIn) + "</sellIn>\n";
        xml += "    <quality>" + std::to_string(MockDatabase::items[i].quality) + "</quality>\n";
        xml += "    <category>" + MockDatabase::items[i].category + "</category>\n";
        xml += "  </item>\n";
    }
    
    xml += "</items>\n";
    return xml;
}

std::string ExportService::exportUsersToCSV() {
    std::stringstream ss;
    ss << "UserId,Username,Role,Status,Contact\n";
    
    for (int i = 0; i < MockDatabase::users.size(); i++) {
        UserDto dto = UserDto::fromEntity(MockDatabase::users[i]);
        ss << dto.publicUserId << ","
           << dto.displayUsername << ","
           << dto.roleName << ","
           << dto.status << ","
           << dto.contactInfo << "\n";
    }
    
    return ss.str();
}
