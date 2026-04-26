#pragma once
#include "models/base/Card.hpp"

class ChanceCard : public Card {
private:
    CardEffectType effectType;
    int value;
    int targetPosition;

public:
    ChanceCard(const std::string& name,
               const std::string& description,
               CardEffectType effectType,
               int value = 0,
               int targetPosition = -1);

    CardType getCardType() const override;
    CardEffectType getEffectType() const;
    int getValue() const;
    int getTargetPosition() const;

    void applyEffect(GameManager& game, Player& player) override;
};
