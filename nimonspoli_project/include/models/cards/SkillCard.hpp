#pragma once
#include <string>
#include "models/base/Card.hpp"

class SkillCard : public Card {
private:
    std::string effectCode;
    int value;
    int remainingDuration;
    bool consumable;

public:
    SkillCard(const std::string& name,
              const std::string& description,
              const std::string& effectCode,
              int value = 0,
              int remainingDuration = 0,
              bool consumable = true);

    CardType getCardType() const override;
    const std::string& getEffectCode() const;
    int getValue() const;
    int getRemainingDuration() const;
    bool isConsumable() const;
    bool isExpired() const;

    void setValue(int newValue);
    void setRemainingDuration(int duration);
    void decrementDuration();

    void applyEffect(GameManager& game, Player& player) override;
};
