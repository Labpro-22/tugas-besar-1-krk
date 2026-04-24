#include "core/game_manager/GameManager.hpp"

#include <stdexcept>

#include "core/services/DebtResolver.hpp"

void GameManager::commandNext() {
    if (turnPhase != TurnPhase::POST_RESOLUTION) {
        throw std::runtime_error("NEXT hanya bisa digunakan setelah resolusi aksi pada giliran ini.");
    }

    if (currentRollWasDouble && !getCurrentPlayer().isInJail() && !getCurrentPlayer().isBankrupt()) {
        turnPhase = TurnPhase::PRE_ROLL;
        currentRollWasDouble = false;
        if (display != 0) {
            display->printMessage("Anda mendapat double. Silakan lempar dadu lagi.");
        }
        return;
    }

    turnPhase = TurnPhase::END_TURN;
    turnShouldEnd = true;
}

void GameManager::commandPayJailFine() {
    Player& player = getCurrentPlayer();
    if (!player.isInJail()) {
        throw std::runtime_error("Pemain sedang tidak di penjara.");
    }
    if (hasRolledThisTurn) {
        throw std::runtime_error("Denda penjara harus dibayar sebelum melempar dadu.");
    }

    DebtResolver debt(*this);
    PaymentRequest request;
    request.amount = getJailFine();
    request.allowDiscount = true;
    request.allowShield = false;
    request.actionName = "PAY_JAIL_FINE";
    request.detail = std::to_string(debt.adjustedAmount(player, getJailFine(), true));
    if (!debt.collect(player, request)) {
        turnPhase = TurnPhase::END_TURN;
        turnShouldEnd = true;
        return;
    }

    player.releaseFromJail();
    turnPhase = TurnPhase::PRE_ROLL;
    if (display != 0) {
        display->printMessage("Denda penjara berhasil dibayar. Silakan lempar dadu.");
    }
}
