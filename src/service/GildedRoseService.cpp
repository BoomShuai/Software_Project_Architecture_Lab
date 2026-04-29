#include "GildedRoseService.h"
#include "../repository/MockDatabase.h"
#include "strategy/ItemStrategy.h"
#include <memory>

void GildedRoseService::dailySettlement() {
    for (auto& item : MockDatabase::items) {
        std::unique_ptr<ItemUpdateStrategy> strategy(ItemStrategyFactory::createStrategy(item.name));
        strategy->update(item);
    }
}
