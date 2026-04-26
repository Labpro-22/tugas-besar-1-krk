#include "models/cards/CommunityChestCard.hpp"
#include "core/services/CardEffectResolver.hpp"
#include "core/game_manager/GameManager.hpp"
#include "models/player/Player.hpp"

CommunityChestCard::CommunityChestCard(const std::string& name,
                                       const std::string& description,
                                       CardEffectType effectType,
                                       int value,
                                       int targetPosition)
    : Card(name, description), effectType(effectType), value(value), targetPosition(targetPosition) {}

CardType CommunityChestCard::getCardType() const { return CardType::COMMUNITY_CHEST; }
CardEffectType CommunityChestCard::getEffectType() const { return effectType; }
int CommunityChestCard::getValue() const { return value; }
int CommunityChestCard::getTargetPosition() const { return targetPosition; }

void CommunityChestCard::applyEffect(GameManager& game, Player& player) {
    CardEffectResolver(game).applyCommunityChestCard(player, *this);
}
