#pragma once
#include <map>
#include "models/base/Property.hpp"

class Utility : public Property {
private:
    std::map<int, int> multiplierByOwnedCount;

public:
    Utility(const std::string& code,
            const std::string& name,
            int position,
            int mortgageValue,
            const std::map<int, int>& multiplierByOwnedCount);

    const std::map<int, int>& getMultiplierByOwnedCount() const;

    int getCurrentRent(int diceTotal) const override;
    int getAssetValue() const override;
    bool hasBuildings() const override;
};
