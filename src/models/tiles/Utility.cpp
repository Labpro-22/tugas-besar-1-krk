#include "models/tiles/Utility.hpp"
#include "models/player/Player.hpp"

Utility::Utility(const std::string &code,
                 const std::string &name,
                 int position,
                 int mortgageValue,
                 const std::map<int, int> &multiplierByOwnedCount)
    : Property(code, name, position, PropertyType::UTILITY, 0, mortgageValue),
      multiplierByOwnedCount(multiplierByOwnedCount) {}

const std::map<int, int> &Utility::getMultiplierByOwnedCount() const { return multiplierByOwnedCount; }

int Utility::getCurrentRent(int diceTotal) const
{
  if (owner == 0)
    return 0;
  int count = owner->countOwnedType(PropertyType::UTILITY);
  std::map<int, int>::const_iterator it = multiplierByOwnedCount.find(count);
  int multiplier = (it != multiplierByOwnedCount.end() ? it->second : 0);
  int rent = diceTotal * multiplier * getFestivalMultiplier();
  return rent;
}

int Utility::getAssetValue() const { return mortgageValue; }
bool Utility::hasBuildings() const { return false; }
