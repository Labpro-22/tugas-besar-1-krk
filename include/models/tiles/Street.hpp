#pragma once
#include <array>
#include <string>
#include "models/base/Property.hpp"

class Street : public Property {
private:
    std::string colorGroup;
    int colorGroupSize;
    int houseCost;
    int hotelCost;
    std::array<int, 6> rentTable;
    int buildingLevel;

public:
    Street(const std::string& code,
           const std::string& name,
           int position,
           const std::string& colorGroup,
           int colorGroupSize,
           int purchasePrice,
           int mortgageValue,
           int houseCost,
           int hotelCost,
           const std::array<int, 6>& rentTable);

    const std::string& getColorGroup() const;
    int getColorGroupSize() const;
    int getBuildingLevel() const;
    int getHouseCost() const;
    int getHotelCost() const;
    const std::array<int, 6>& getRentTable() const;

    bool isMonopoly() const;
    bool canBuild() const;
    bool canSellBuilding() const;

    void build();
    void sellOneBuilding();
    void resetBuildings();
    void setBuildingLevel(int level);

    int getCurrentRent(int diceTotal) const override;
    int getAssetValue() const override;
    bool hasBuildings() const override;
};
