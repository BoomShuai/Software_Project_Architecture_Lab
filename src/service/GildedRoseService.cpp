#include "GildedRoseService.h"
#include "../repository/MockDatabase.h"
#include "strategy/ItemStrategy.h"
#include "../utils/Logger.h"
#include <memory>
#include <cassert>

void GildedRoseService::dailySettlement() {
    Logger::logInfo("Daily settlement started. Item count: "
                    + std::to_string(MockDatabase::items.size()), "GildedRoseService");

    for (auto& item : MockDatabase::items) {
        int oldQuality = item.quality;
        int oldSellIn = item.sellIn;

        std::unique_ptr<ItemUpdateStrategy> strategy(ItemStrategyFactory::createStrategy(item.name));

        // Pre-condition: strategy must be valid
        assert(strategy != nullptr && "Factory must return a valid strategy");

        strategy->update(item);

        Logger::logDebug("Settlement applied: [" + item.name + "] sellIn: "
                         + std::to_string(oldSellIn) + " -> " + std::to_string(item.sellIn)
                         + ", quality: " + std::to_string(oldQuality) + " -> " + std::to_string(item.quality),
                         "GildedRoseService");
    }

    Logger::logInfo("Daily settlement completed.", "GildedRoseService");
}
