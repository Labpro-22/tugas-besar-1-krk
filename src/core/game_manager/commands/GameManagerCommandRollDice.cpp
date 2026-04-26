#include "core/game_manager/GameManager.hpp"

#include <stdexcept>

#include "../GameManagerInternal.hpp"
#include "models/base/Tile.hpp"

std::pair<int, int> GameManager::commandRollDice() {
    if (turnPhase == TurnPhase::JAIL_CHOICE) {
        throw std::runtime_error("Anda wajib keluar dari penjara terlebih dahulu dengan BAYAR_DENDA.");
    }
    if (turnPhase != TurnPhase::PRE_ROLL) {
        throw std::runtime_error("Gunakan NEXT untuk melanjutkan giliran terlebih dahulu.");
    }

    std::pair<int, int> result = rollMovementDice();
    int total = result.first + result.second;
    Player& player = getCurrentPlayer();
    hasRolledThisTurn = true;
    currentRollWasDouble = (result.first == result.second);

    if (display != 0) {
        display->printMessage(useManualDice ? "Dadu diatur secara manual." : "Mengocok dadu...");
        display->printMessage("Hasil: " + std::to_string(result.first) + " + " + std::to_string(result.second) + " = " + std::to_string(total));
    }

    if (player.isInJail()) {
        if (result.first == result.second) {
            player.releaseFromJail();
            ++consecutiveDoubles;
            movePlayer(player, total);
            if (display != 0) {
                Tile* landed = board.getTile(player.getPosition());
                display->printMessage("Bidak keluar dari penjara dan mendarat di: "
                    + GameManagerInternal::prettyName(landed != 0 ? landed->getName() : ""));
            }
            resolveLanding(player, total);
            if (player.isInJail() || player.isBankrupt()) {
                currentRollWasDouble = false;
            }
        } else {
            player.incrementJailTurns();
            currentRollWasDouble = false;
            if (display != 0) {
                display->printMessage("Gagal mendapatkan double. Pemain tetap di penjara.");
            }
        }

        turnPhase = TurnPhase::POST_RESOLUTION;
        return result;
    }

    if (currentRollWasDouble) {
        ++consecutiveDoubles;
        if (consecutiveDoubles >= 3) {
            handleJail(player);
            currentRollWasDouble = false;
            turnPhase = TurnPhase::END_TURN;
            turnShouldEnd = true;
            if (display != 0) {
                display->printMessage("Three doubles in a row. Go to jail.");
            }
            return result;
        }
    } else {
        consecutiveDoubles = 0;
    }

    if (display != 0) {
        display->printMessage("Memajukan Bidak " + player.getName() + " sebanyak " + std::to_string(total) + " petak...");
    }
    movePlayer(player, total);
    if (display != 0) {
        Tile* landed = board.getTile(player.getPosition());
        display->printMessage("Bidak mendarat di: "
            + GameManagerInternal::prettyName(landed != 0 ? landed->getName() : ""));
    }
    resolveLanding(player, total);
    if (player.isInJail() || player.isBankrupt()) {
        currentRollWasDouble = false;
    }
    turnPhase = player.isBankrupt() ? TurnPhase::END_TURN : TurnPhase::POST_RESOLUTION;
    turnShouldEnd = player.isBankrupt();
    return result;
}

void GameManager::commandSetDice(int die1, int die2) {
    if (die1 < 1 || die1 > 6 || die2 < 1 || die2 > 6) {
        throw std::runtime_error("Nilai dadu manual harus berada pada rentang 1-6.");
    }

    setManualDice(die1, die2);
    try {
        commandRollDice();
    } catch (...) {
        clearManualDice();
        throw;
    }
    clearManualDice();
}
