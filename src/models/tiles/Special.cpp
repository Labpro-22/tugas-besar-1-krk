#include "models/tiles/Special.hpp"
#include "core/game_manager/GameManager.hpp"
#include "models/player/Player.hpp"

Special::Special(const std::string& code,
                 const std::string& name,
                 int position,
                 SpecialType specialType)
    : Tile(code, name, position), specialType(specialType) {}

SpecialType Special::getSpecialType() const { return specialType; }

void Special::onLanded(GameManager& game, Player& player, int /*diceTotal*/) {
    switch (specialType) {
        case SpecialType::GO:
            break;
        case SpecialType::JAIL:
            // Just visiting.
            break;
        case SpecialType::FREE_PARKING:
            break;
        case SpecialType::GO_TO_JAIL:
            game.handleJail(player);
            break;
    }
}

std::string Special::getDisplayType() const {
    return "SPECIAL";
}
