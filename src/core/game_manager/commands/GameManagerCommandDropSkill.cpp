#include "core/game_manager/GameManager.hpp"

#include <stdexcept>

void GameManager::commandDropSkillCard(std::size_t skillCardIndex) {
    Player& player = getCurrentPlayer();
    if (skillCardIndex >= player.getInventory().getSkillCardCount()) {
        throw std::runtime_error("Invalid skill card index.");
    }

    SkillCard* card = player.getInventory().takeSkillCard(skillCardIndex);
    logger.log(currentTurn, player.getName(), "DROP_SKILL", card->getName());
    discardSkillCard(card);
}
