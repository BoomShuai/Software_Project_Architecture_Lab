#ifndef ITEM_H
#define ITEM_H

#include <string>

struct Item {
    int id;
    std::string name;
    int sellIn;
    int quality;
    std::string description;
    std::string category;
    long long createdAt;
    long long updatedAt;
    bool isSpecial;
    int weight;
    std::string owner;

    Item() : id(0), name(""), sellIn(0), quality(0), description(""), category(""), createdAt(0), updatedAt(0), isSpecial(false), weight(0), owner("") {}
    
    Item(int id, std::string name, int sellIn, int quality) 
        : id(id), name(name), sellIn(sellIn), quality(quality), description("Default item"), category("General"), createdAt(1600000000), updatedAt(1600000000), isSpecial(false), weight(1), owner("System") {}
        
    Item(int id, std::string name, int sellIn, int quality, std::string desc, std::string cat) 
        : id(id), name(name), sellIn(sellIn), quality(quality), description(desc), category(cat), createdAt(1600000000), updatedAt(1600000000), isSpecial(false), weight(1), owner("System") {}
};

#endif // ITEM_H
