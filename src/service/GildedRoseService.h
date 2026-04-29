#ifndef GILDED_ROSE_SERVICE_H
#define GILDED_ROSE_SERVICE_H

/**
 * @class GildedRoseService
 * @brief Manages the daily inventory settlement process.
 * 
 * Implements the core business logic using the Strategy Pattern
 * to update the Quality and SellIn values of all items at the end of each day.
 */
class GildedRoseService {
public:
    /**
     * @brief Executes the daily settlement logic.
     * 
     * Iterates through all items in the inventory and applies
     * the specific ItemUpdateStrategy based on the item's name.
     */
    void dailySettlement();
};

#endif // GILDED_ROSE_SERVICE_H
