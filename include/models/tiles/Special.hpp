#pragma once
#include "models/base/GameTypes.hpp"
#include "models/base/Tile.hpp"

class Special : public Tile {
private:
    SpecialType specialType;

public:
    Special(const std::string& code,
            const std::string& name,
            int position,
            SpecialType specialType);

    SpecialType getSpecialType() const;

    void onLanded(GameManager& game, Player& player, int diceTotal) override;
    std::string getDisplayType() const override;
};
