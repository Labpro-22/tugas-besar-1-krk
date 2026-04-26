#include "models/tiles/Action.hpp"
#include "core/game_manager/GameManager.hpp"

Action::Action(const std::string& code,
               const std::string& name,
               int position,
               ActionType actionType,
               int fixedAmount)
    : Tile(code, name, position), actionType(actionType), fixedAmount(fixedAmount) {}

ActionType Action::getActionType() const { return actionType; }
int Action::getFixedAmount() const { return fixedAmount; }

void Action::onLanded(GameManager& game, Player& player, int /*diceTotal*/) {
    switch (actionType) {
        case ActionType::CHANCE:
            game.drawChanceCard(player);
            break;
        case ActionType::COMMUNITY_CHEST:
            game.drawCommunityChestCard(player);
            break;
        case ActionType::FESTIVAL:
            game.handleFestival(player);
            break;
        case ActionType::TAX_PPH:
        case ActionType::TAX_PBM:
            game.handleTax(player, actionType);
            break;
    }
}

std::string Action::getDisplayType() const {
    return "ACTION";
}
