#include "core/game_manager/GameManager.hpp"

#include <sstream>
#include <stdexcept>

void GameManager::gameLoop() {
    while (!isGameOver()) {
        playTurn();
    }
    commandAnnounceWinner();
}

void GameManager::playTurn() {
    Player& player = getCurrentPlayer();
    if (player.isBankrupt()) {
        endTurn();
        return;
    }

    refreshFestivalStates(player);
    hasRolledThisTurn = false;
    hasUsedSkillThisTurn = false;
    consecutiveDoubles = 0;
    currentRollWasDouble = false;
    turnShouldEnd = false;
    awardSkillCard(player);
    turnPhase = (player.isInJail() && player.getJailTurnsSpent() >= 3)
        ? TurnPhase::JAIL_CHOICE
        : TurnPhase::PRE_ROLL;

    if (display != 0) {
        std::ostringstream oss;
        oss << "\n=== Turn " << currentTurn << " | Player: " << player.getName() << " ===";
        display->printMessage(oss.str());
    }

    while (!turnShouldEnd && !player.isBankrupt()) {
        std::string line;
        if (display != 0) {
            line = display->getInput("> ");
        } else if (!hasRolledThisTurn) {
            line = (player.isInJail() && player.getJailTurnsSpent() >= 3) ? "BAYAR_DENDA" : "LEMPAR_DADU";
        } else {
            line = "NEXT";
        }

        if (line.empty()) {
            continue;
        }

        try {
            handleCommand(line);
        } catch (const std::exception& e) {
            if (display != 0) {
                display->printMessage(std::string("Error: ") + e.what());
            }
        }
    }

    endTurn();
}

void GameManager::endTurn() {
    tickPlayerSkillDurations(getCurrentPlayer());
    if (players.empty()) {
        return;
    }

    int startIndex = currentPlayerIndex;
    do {
        int nextIndex = (currentPlayerIndex + 1) % static_cast<int>(players.size());
        if (nextIndex == 0) {
            ++currentTurn;
        }
        currentPlayerIndex = nextIndex;
        if (!players[static_cast<std::size_t>(currentPlayerIndex)]->isBankrupt()) {
            break;
        }
    } while (currentPlayerIndex != startIndex);

    hasRolledThisTurn = false;
    hasUsedSkillThisTurn = false;
    consecutiveDoubles = 0;
    currentRollWasDouble = false;
    turnShouldEnd = false;
    turnPhase = TurnPhase::START_TURN;
}
