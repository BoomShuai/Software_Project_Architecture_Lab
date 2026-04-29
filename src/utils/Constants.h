#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

namespace Constants {
    const std::string AGED_BRIE = "Aged Brie";
    const std::string SULFURAS = "Sulfuras, Hand of Ragnaros";
    const std::string BACKSTAGE_PASS = "Backstage passes to a TAFKAL80ETC concert";
    const std::string CONJURED = "Conjured Mana Cake";
    
    const int MAX_QUALITY = 50;
    const int MIN_QUALITY = 0;
    const int SULFURAS_QUALITY = 80;
    
    const int SELL_IN_WARNING = 3;
    const int QUALITY_WARNING = 5;
    
    const int BACKSTAGE_THRESHOLD_1 = 11;
    const int BACKSTAGE_THRESHOLD_2 = 6;
    
    const std::string TOKEN_PREFIX = "Bearer";
    const int MIN_TOKEN_LEN = 10;
}

#endif
