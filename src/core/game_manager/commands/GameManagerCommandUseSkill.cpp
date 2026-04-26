#include "core/game_manager/GameManager.hpp"

#include <stdexcept>

void GameManager::commandUseSkillCard(std::size_t skillCardIndex) {
    Player& player = getCurrentPlayer();
    if (hasUsedSkillThisTurn) {
        throw std::runtime_error("Only one skill card may be used per turn.");
    }
    if (hasRolledThisTurn) {
        throw std::runtime_error("Skill cards must be used before rolling.");
    }
    if (skillCardIndex >= player.getInventory().getSkillCardCount()) {
        throw std::runtime_error("Invalid skill card index.");
    }

    SkillCard* card = player.getInventory().takeSkillCard(skillCardIndex);
    const bool wasInJail = player.isInJail();
    card->applyEffect(*this, player);
    logger.log(currentTurn, player.getName(), "USE_SKILL", card->getName());
    hasUsedSkillThisTurn = true;

    if (player.isBankrupt()) {
        turnPhase = TurnPhase::END_TURN;
        turnShouldEnd = true;
    } else if (!wasInJail && player.isInJail()) {
        turnPhase = TurnPhase::POST_RESOLUTION;
        currentRollWasDouble = false;
    }

    discardSkillCard(card);
}
