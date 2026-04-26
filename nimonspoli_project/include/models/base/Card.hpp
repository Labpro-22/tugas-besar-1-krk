#pragma once
#include <string>
#include "models/base/GameTypes.hpp"

class GameManager;
class Player;

class Card {
protected:
    std::string name;
    std::string description;

public:
    Card(const std::string& name, const std::string& description);
    virtual ~Card() = default;

    const std::string& getName() const;
    const std::string& getDescription() const;

    virtual CardType getCardType() const = 0;
    virtual void applyEffect(GameManager& game, Player& player) = 0;
};
