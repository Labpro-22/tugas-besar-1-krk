#pragma once

#include "models/base/GameTypes.hpp"

class ChanceCard;
class CommunityChestCard;
class GameManager;
class Player;
class SkillCard;

class CardEffectResolver {
private:
    GameManager& game;

    void applyCommonCardEffect(Player& player,
                               const char* sourceAction,
                               CardEffectType effectType,
                               int value,
                               int targetPosition);
    void applyMoveToNearestRailroad(Player& player);
    void applyTeleport(Player& player);
    void applyLasso(Player& player);
    void applyDemolition(Player& player);

public:
    explicit CardEffectResolver(GameManager& game);

    void applyChanceCard(Player& player, const ChanceCard& card);
    void applyCommunityChestCard(Player& player, const CommunityChestCard& card);
    void applySkillCard(Player& player, SkillCard& card);
};
