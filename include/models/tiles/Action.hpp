#pragma once
#include "models/base/GameTypes.hpp"
#include "models/base/Tile.hpp"

class Action : public Tile {
private:
    ActionType actionType;
    int fixedAmount;

public:
    Action(const std::string& code,
           const std::string& name,
           int position,
           ActionType actionType,
           int fixedAmount = 0);

    ActionType getActionType() const;
    int getFixedAmount() const;

    void onLanded(GameManager& game, Player& player, int diceTotal) override;
    std::string getDisplayType() const override;
};
