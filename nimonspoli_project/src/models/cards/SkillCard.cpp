#include "models/cards/SkillCard.hpp"
#include "core/services/CardEffectResolver.hpp"
#include "core/game_manager/GameManager.hpp"
#include "models/player/Player.hpp"

SkillCard::SkillCard(const std::string& name,
                     const std::string& description,
                     const std::string& effectCode,
                     int value,
                     int remainingDuration,
                     bool consumable)
    : Card(name, description),
      effectCode(effectCode),
      value(value),
      remainingDuration(remainingDuration),
      consumable(consumable) {}

CardType SkillCard::getCardType() const { return CardType::SKILL; }
const std::string& SkillCard::getEffectCode() const { return effectCode; }
int SkillCard::getValue() const { return value; }
int SkillCard::getRemainingDuration() const { return remainingDuration; }
bool SkillCard::isConsumable() const { return consumable; }
bool SkillCard::isExpired() const { return remainingDuration <= 0 && !consumable; }

void SkillCard::setValue(int newValue) { value = newValue; }
void SkillCard::setRemainingDuration(int duration) { remainingDuration = duration; }
void SkillCard::decrementDuration() { if (remainingDuration > 0) --remainingDuration; }

void SkillCard::applyEffect(GameManager& game, Player& player) {
    CardEffectResolver(game).applySkillCard(player, *this);
}
