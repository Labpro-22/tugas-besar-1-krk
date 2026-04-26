#include "models/tiles/Street.hpp"
#include <stdexcept>
#include "models/player/Player.hpp"

Street::Street(const std::string &code,
               const std::string &name,
               int position,
               const std::string &colorGroup,
               int colorGroupSize,
               int purchasePrice,
               int mortgageValue,
               int houseCost,
               int hotelCost,
               const std::array<int, 6> &rentTable)
    : Property(code, name, position, PropertyType::STREET, purchasePrice, mortgageValue),
      colorGroup(colorGroup),
      colorGroupSize(colorGroupSize),
      houseCost(houseCost),
      hotelCost(hotelCost),
      rentTable(rentTable),
      buildingLevel(0) {}

const std::string &Street::getColorGroup() const { return colorGroup; }
int Street::getColorGroupSize() const { return colorGroupSize; }
int Street::getBuildingLevel() const { return buildingLevel; }
int Street::getHouseCost() const { return houseCost; }
int Street::getHotelCost() const { return hotelCost; }
const std::array<int, 6> &Street::getRentTable() const { return rentTable; }

bool Street::isMonopoly() const
{
    return owner != 0 && owner->countOwnedColorGroup(colorGroup) >= colorGroupSize;
}

bool Street::canBuild() const
{
    return !isMortgaged() && isMonopoly() && buildingLevel < 5;
}

bool Street::canSellBuilding() const
{
    return buildingLevel > 0;
}

void Street::build()
{
    if (!canBuild())
    {
        throw std::runtime_error("Street cannot be built right now.");
    }
    ++buildingLevel;
}

void Street::sellOneBuilding()
{
    if (!canSellBuilding())
    {
        throw std::runtime_error("Street has no building to sell.");
    }
    --buildingLevel;
}

void Street::resetBuildings() { buildingLevel = 0; }

void Street::setBuildingLevel(int level)
{
    if (level < 0)
        level = 0;
    if (level > 5)
        level = 5;
    buildingLevel = level;
}

int Street::getCurrentRent(int /*diceTotal*/) const
{
    int base = rentTable[static_cast<std::size_t>(buildingLevel)];
    if (buildingLevel == 0 && isMonopoly())
    {
        base *= 2;
    }
    base *= getFestivalMultiplier();
    return base;
}

int Street::getAssetValue() const
{
    int value = purchasePrice;
    for (int i = 0; i < buildingLevel; ++i)
    {
        value += (i < 4 ? houseCost : hotelCost);
    }
    return value;
}

bool Street::hasBuildings() const { return buildingLevel > 0; }
