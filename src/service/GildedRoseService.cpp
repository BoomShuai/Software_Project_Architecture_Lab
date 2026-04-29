#include "GildedRoseService.h"
#include "../repository/MockDatabase.h"
#include <vector>
#include <iostream>

using namespace std;

void GildedRoseService::dailySettlement() {
    // using the exact original code logic
    for (int i = 0; i < MockDatabase::items.size(); i++)
    {
        if (MockDatabase::items[i].name != "Aged Brie" && MockDatabase::items[i].name != "Backstage passes to a TAFKAL80ETC concert")
        {
            if (MockDatabase::items[i].quality > 0)
            {
                if (MockDatabase::items[i].name != "Sulfuras, Hand of Ragnaros")
                {
                    MockDatabase::items[i].quality = MockDatabase::items[i].quality - 1;
                }
            }
        }
        else
        {
            if (MockDatabase::items[i].quality < 50)
            {
                MockDatabase::items[i].quality = MockDatabase::items[i].quality + 1;

                if (MockDatabase::items[i].name == "Backstage passes to a TAFKAL80ETC concert")
                {
                    if (MockDatabase::items[i].sellIn < 11)
                    {
                        if (MockDatabase::items[i].quality < 50)
                        {
                            MockDatabase::items[i].quality = MockDatabase::items[i].quality + 1;
                        }
                    }

                    if (MockDatabase::items[i].sellIn < 6)
                    {
                        if (MockDatabase::items[i].quality < 50)
                        {
                            MockDatabase::items[i].quality = MockDatabase::items[i].quality + 1;
                        }
                    }
                }
            }
        }

        if (MockDatabase::items[i].name != "Sulfuras, Hand of Ragnaros")
        {
            MockDatabase::items[i].sellIn = MockDatabase::items[i].sellIn - 1;
        }

        if (MockDatabase::items[i].sellIn < 0)
        {
            if (MockDatabase::items[i].name != "Aged Brie")
            {
                if (MockDatabase::items[i].name != "Backstage passes to a TAFKAL80ETC concert")
                {
                    if (MockDatabase::items[i].quality > 0)
                    {
                        if (MockDatabase::items[i].name != "Sulfuras, Hand of Ragnaros")
                        {
                            MockDatabase::items[i].quality = MockDatabase::items[i].quality - 1;
                        }
                    }
                }
                else
                {
                    MockDatabase::items[i].quality = MockDatabase::items[i].quality - MockDatabase::items[i].quality;
                }
            }
            else
            {
                if (MockDatabase::items[i].quality < 50)
                {
                    MockDatabase::items[i].quality = MockDatabase::items[i].quality + 1;
                }
            }
        }
    }
}
