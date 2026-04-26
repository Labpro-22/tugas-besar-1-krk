#pragma once
#include <map>
#include "models/base/Property.hpp"

class Railroad : public Property
{
private:
    std::map<int, int> rentByOwnedCount;

public:
    Railroad(const std::string &code,
             const std::string &name,
             int position,
             int mortgageValue,
             const std::map<int, int> &rentByOwnedCount);

    const std::map<int, int> &getRentByOwnedCount() const;

    int getCurrentRent(int diceTotal) const override;
    int getAssetValue() const override;
    bool hasBuildings() const override;
};
