#include "models/cards/ChanceCard.hpp"
#include "core/services/CardEffectResolver.hpp"
#include "core/game_manager/GameManager.hpp"
#include "models/player/Player.hpp"

ChanceCard::ChanceCard(const std::string& name,
                       const std::string& description,
                       CardEffectType effectType,
                       int value,
                       int targetPosition)
    : Card(name, description), effectType(effectType), value(value), targetPosition(targetPosition) {}

CardType ChanceCard::getCardType() const { return CardType::CHANCE; }
CardEffectType ChanceCard::getEffectType() const { return effectType; }
int ChanceCard::getValue() const { return value; }
int ChanceCard::getTargetPosition() const { return targetPosition; }

void ChanceCard::applyEffect(GameManager& game, Player& player) {
    CardEffectResolver(game).applyChanceCard(player, *this);
}
