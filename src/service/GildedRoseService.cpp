#include "GildedRoseService.h"
#include "../repository/MockDatabase.h"
#include "strategy/ItemStrategy.h"
#include "../utils/Logger.h"
#include "../utils/ThreadPool.h"
#include <memory>
#include <cassert>
#include <future>
#include <vector>

void GildedRoseService::dailySettlement() {
    Logger::logInfo("Daily settlement started. Item count: "
                    + std::to_string(MockDatabase::items.size()), "GildedRoseService");

    for (auto& item : MockDatabase::items) {
        int oldQuality = item.quality;
        int oldSellIn = item.sellIn;

        // OPTIMIZED: Use pooled strategy (shared_ptr, no new/delete per iteration)
        std::shared_ptr<ItemUpdateStrategy> strategy = ItemStrategyFactory::getStrategy(item.name);

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

void GildedRoseService::dailySettlementParallel(int numThreads) {
    Logger::logInfo("Parallel daily settlement started. Item count: "
                    + std::to_string(MockDatabase::items.size())
                    + ", Threads: " + std::to_string(numThreads), "GildedRoseService");

    size_t totalItems = MockDatabase::items.size();
    if (totalItems == 0) return;

    // Partition items into chunks for each thread
    size_t chunkSize = (totalItems + numThreads - 1) / numThreads;

    {
        ThreadPool pool(numThreads);
        std::vector<std::future<void>> futures;

        for (int t = 0; t < numThreads; t++) {
            size_t start = t * chunkSize;
            size_t end = std::min(start + chunkSize, totalItems);
            if (start >= totalItems) break;

            futures.push_back(pool.submit([start, end]() {
                // Each thread processes its own slice of the items vector.
                // No data races because each thread writes to distinct indices.
                for (size_t i = start; i < end; i++) {
                    Item& item = MockDatabase::items[i];
                    // OPTIMIZED: Pooled strategy — thread-safe since strategies are stateless
                    auto strategy = ItemStrategyFactory::getStrategy(item.name);
                    strategy->update(item);
                }
            }));
        }

        // Wait for all chunks to complete
        for (auto& f : futures) {
            f.get();
        }
    } // ThreadPool destructor joins all threads

    Logger::logInfo("Parallel daily settlement completed.", "GildedRoseService");
}
