#ifndef ITEM_STRATEGY_H
#define ITEM_STRATEGY_H

#include "../../model/Item.h"
#include "../../utils/Constants.h"
#include <algorithm>

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

class ItemStrategyFactory {
public:
    static ItemUpdateStrategy* createStrategy(const std::string& itemName) {
        if (itemName == Constants::AGED_BRIE) return new AgedBrieStrategy();
        if (itemName == Constants::SULFURAS) return new SulfurasStrategy();
        if (itemName == Constants::BACKSTAGE_PASS) return new BackstagePassStrategy();
        return new NormalItemStrategy();
    }
};

#endif
