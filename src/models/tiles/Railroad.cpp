#include "models/tiles/Railroad.hpp"
#include "models/player/Player.hpp"

Railroad::Railroad(const std::string &code,
                   const std::string &name,
                   int position,
                   int mortgageValue,
                   const std::map<int, int> &rentByOwnedCount)
    : Property(code, name, position, PropertyType::RAILROAD, 0, mortgageValue),
      rentByOwnedCount(rentByOwnedCount) {}

const std::map<int, int> &Railroad::getRentByOwnedCount() const { return rentByOwnedCount; }

int Railroad::getCurrentRent(int /*diceTotal*/) const
{
  if (owner == 0)
    return 0;
  int count = owner->countOwnedType(PropertyType::RAILROAD);
  std::map<int, int>::const_iterator it = rentByOwnedCount.find(count);
  int rent = (it != rentByOwnedCount.end() ? it->second : 0);
  rent *= getFestivalMultiplier();
  return rent;
}

int Railroad::getAssetValue() const { return mortgageValue; }
bool Railroad::hasBuildings() const { return false; }
