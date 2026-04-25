#include "core/foundation/Board.hpp"
#include "models/base/Property.hpp"
#include "models/tiles/Street.hpp"
#include "models/base/Tile.hpp"

Board::Board() {}

Board::~Board()
{
    clear();
}

void Board::addTile(Tile *tile)
{
    tiles.push_back(tile);
}

void Board::clear()
{
    for (std::size_t i = 0; i < tiles.size(); ++i)
    {
        delete tiles[i];
    }
    tiles.clear();
}

int Board::size() const
{
    return static_cast<int>(tiles.size());
}

Tile *Board::getTile(int position) const
{
    if (position < 1 || position > size())
    {
        return 0;
    }
    return tiles[static_cast<std::size_t>(position - 1)];
}

Property *Board::getPropertyByCode(const std::string &code) const
{
    for (std::size_t i = 0; i < tiles.size(); ++i)
    {
        Property *property = dynamic_cast<Property *>(tiles[i]);
        if (property != 0 && property->getCode() == code)
        {
            return property;
        }
    }
    return 0;
}

std::vector<Property *> Board::getAllProperties() const
{
    std::vector<Property *> result;
    for (std::size_t i = 0; i < tiles.size(); ++i)
    {
        Property *property = dynamic_cast<Property *>(tiles[i]);
        if (property != 0)
        {
            result.push_back(property);
        }
    }
    return result;
}

std::vector<Property *> Board::getPropertiesByColorGroup(const std::string &colorGroup) const
{
    std::vector<Property *> result;
    for (std::size_t i = 0; i < tiles.size(); ++i)
    {
        Street *street = dynamic_cast<Street *>(tiles[i]);
        if (street != 0 && street->getColorGroup() == colorGroup)
        {
            result.push_back(street);
        }
    }
    return result;
}
