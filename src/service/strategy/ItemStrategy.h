#ifndef ITEM_STRATEGY_H
#define ITEM_STRATEGY_H

#include "../../model/Item.h"
#include "../../utils/Constants.h"
#include <algorithm>
#include <memory>

class ItemUpdateStrategy {
public:
    virtual ~ItemUpdateStrategy() = default;
    virtual void update(Item& item) = 0;
protected:
    void increaseQuality(Item& item, int amount = 1) {
        item.quality = std::min(item.quality + amount, Constants::MAX_QUALITY);
    }
    void decreaseQuality(Item& item, int amount = 1) {
        item.quality = std::max(item.quality - amount, Constants::MIN_QUALITY);
    }
    void decreaseSellIn(Item& item) {
        item.sellIn -= 1;
    }
};

class NormalItemStrategy : public ItemUpdateStrategy {
public:
    void update(Item& item) override {
        decreaseSellIn(item);
        decreaseQuality(item);
        if (item.sellIn < 0) {
            decreaseQuality(item);
        }
    }
};

class AgedBrieStrategy : public ItemUpdateStrategy {
public:
    void update(Item& item) override {
        decreaseSellIn(item);
        increaseQuality(item);
        if (item.sellIn < 0) {
            increaseQuality(item);
        }
    }
};

class SulfurasStrategy : public ItemUpdateStrategy {
public:
    void update(Item& item) override {
        // Sulfuras neither decreases in quality nor sellIn
    }
};

class BackstagePassStrategy : public ItemUpdateStrategy {
public:
    void update(Item& item) override {
        decreaseSellIn(item);
        increaseQuality(item);
        if (item.sellIn < Constants::BACKSTAGE_THRESHOLD_1) {
            increaseQuality(item);
        }
        if (item.sellIn < Constants::BACKSTAGE_THRESHOLD_2) {
            increaseQuality(item);
        }
        if (item.sellIn < 0) {
            item.quality = Constants::MIN_QUALITY;
        }
    }
};

/**
 * @class ItemStrategyFactory
 * @brief Factory for creating item update strategies.
 *
 * Performance optimization (Experiment 3):
 * - createStrategy(): original API, allocates new object each call (for backward compatibility)
 * - getStrategy(): OPTIMIZED — returns shared_ptr to pooled static objects,
 *   avoiding heap allocation/deallocation per item in hot loops.
 *   Strategy objects are stateless, so sharing is safe.
 */
class ItemStrategyFactory {
public:
    /// Original API: creates a new strategy on each call (backward compatible)
    static ItemUpdateStrategy* createStrategy(const std::string& itemName) {
        if (itemName == Constants::AGED_BRIE) return new AgedBrieStrategy();
        if (itemName == Constants::SULFURAS) return new SulfurasStrategy();
        if (itemName == Constants::BACKSTAGE_PASS) return new BackstagePassStrategy();
        return new NormalItemStrategy();
    }

    /**
     * @brief OPTIMIZED: Returns a shared pointer to a pooled strategy object.
     *
     * Since all strategy classes are stateless (no member variables), they can
     * be safely shared across calls and even threads. This eliminates the
     * new/delete overhead in hot loops like dailySettlement().
     *
     * @param itemName The item name to determine strategy type.
     * @return Shared pointer to the appropriate strategy (pooled, not newly allocated).
     */
    static std::shared_ptr<ItemUpdateStrategy> getStrategy(const std::string& itemName) {
        // Static pooled instances — created once, reused forever
        static auto normalStrategy = std::make_shared<NormalItemStrategy>();
        static auto agedBrieStrategy = std::make_shared<AgedBrieStrategy>();
        static auto sulfurasStrategy = std::make_shared<SulfurasStrategy>();
        static auto backstageStrategy = std::make_shared<BackstagePassStrategy>();

        if (itemName == Constants::AGED_BRIE) return agedBrieStrategy;
        if (itemName == Constants::SULFURAS) return sulfurasStrategy;
        if (itemName == Constants::BACKSTAGE_PASS) return backstageStrategy;
        return normalStrategy;
    }
};

#endif
