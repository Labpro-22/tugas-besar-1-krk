#pragma once
#include <string>
#include <vector>
using namespace std;

class Property;
class Tile;

class Board
{
private:
    vector<Tile *> tiles;

public:
    Board();
    Board(const Board &) = delete;
    Board &operator=(const Board &) = delete;
    ~Board();

    void addTile(Tile *tile);
    void clear();

    int size() const;
    Tile *getTile(int position) const;

    Property *getPropertyByCode(const string &code) const;
    std::vector<Property *> getAllProperties() const;
    std::vector<Property *> getPropertiesByColorGroup(const string &colorGroup) const;
};
