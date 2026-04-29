#ifndef ITEM_DTO_H
#define ITEM_DTO_H

#include <string>
#include "../model/Item.h"

struct ItemDto {
    std::string itemIdStr;
    std::string displayName;
    std::string daysRemaining;
    std::string itemQuality;
    std::string fullDescription;
    std::string auditInfo;
    
    static ItemDto fromEntity(const Item& item) {
        ItemDto dto;
        dto.itemIdStr = "ITEM_" + std::to_string(item.id);
        dto.displayName = "[Gilded Rose] " + item.name;
        dto.daysRemaining = std::to_string(item.sellIn) + " days";
        dto.itemQuality = std::to_string(item.quality) + " points";
        dto.fullDescription = item.category + " - " + item.description;
        dto.auditInfo = "Created: " + std::to_string(item.createdAt) + " | By: " + item.owner;
        return dto;
    }
};

#endif // ITEM_DTO_H
