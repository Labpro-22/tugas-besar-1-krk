#include "core/services/CardEffectResolver.hpp"

#include <cctype>
#include <climits>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include "core/services/DebtResolver.hpp"
#include "core/game_manager/GameManager.hpp"
#include "../game_manager/GameManagerInternal.hpp"
#include "models/cards/ChanceCard.hpp"
#include "models/cards/CommunityChestCard.hpp"
#include "models/player/Player.hpp"
#include "models/tiles/Railroad.hpp"
#include "models/cards/SkillCard.hpp"
#include "models/tiles/Street.hpp"
#include "models/base/Tile.hpp"

namespace {

Tile* findTileByCode(const Board& board, const std::string& code) {
    for (int position = 1; position <= board.size(); ++position) {
        Tile* tile = board.getTile(position);
        if (tile != 0 && GameManagerInternal::toUpper(tile->getCode()) == GameManagerInternal::toUpper(code)) {
            return tile;
        }
    }
    return 0;
}

}

CardEffectResolver::CardEffectResolver(GameManager& game) : game(game) {}

void CardEffectResolver::applyCommonCardEffect(Player& player,
                                               const char* sourceAction,
                                               CardEffectType effectType,
                                               int value,
                                               int targetPosition) {
    DebtResolver debt(game);
    switch (effectType) {
        case CardEffectType::RECEIVE_MONEY:
            game.bank.payPlayer(player, value);
            game.logger.log(game.currentTurn, player.getName(), sourceAction, "TERIMA=" + std::to_string(value));
            break;
        case CardEffectType::PAY_MONEY: {
            PaymentRequest request;
            request.amount = value;
            request.allowDiscount = true;
            request.allowShield = true;
            request.actionName = sourceAction;
            request.detail = "BAYAR=" + std::to_string(value);
            request.blockDetail = "CARD";
            debt.collect(player, request);
            break;
        }
        case CardEffectType::MOVE_RELATIVE:
            game.movePlayer(player, value);
            game.resolveLanding(player, 0);
            game.logger.log(game.currentTurn, player.getName(), sourceAction,
                            "PINDAH_RELATIF=" + std::to_string(value));
            break;
        case CardEffectType::MOVE_TO_POSITION:
            game.movePlayerTo(player, targetPosition, true);
            game.resolveLanding(player, 0);
            game.logger.log(game.currentTurn, player.getName(), sourceAction,
                            "PINDAH=" + std::to_string(targetPosition));
            break;
        case CardEffectType::MOVE_TO_NEAREST_RAILROAD:
            applyMoveToNearestRailroad(player);
            break;
        case CardEffectType::GO_TO_JAIL:
            game.handleJail(player);
            break;
        case CardEffectType::GET_OUT_OF_JAIL:
            player.releaseFromJail();
            game.logger.log(game.currentTurn, player.getName(), sourceAction, "KELUAR_PENJARA");
            break;
        case CardEffectType::PAY_EACH_PLAYER: {
            const std::vector<Player*>& players = game.getPlayers();
            int opponentCount = 0;
            for (std::size_t i = 0; i < players.size(); ++i) {
                if (players[i] != &player && !players[i]->isBankrupt()) {
                    ++opponentCount;
                }
            }
            const int perPlayerAmount = debt.adjustedAmount(player, value, true);
            PaymentRequest request;
            request.amount = perPlayerAmount * opponentCount;
            request.allowDiscount = false;
            request.allowShield = true;
            request.actionName = sourceAction;
            request.detail = "BAYAR_SEMUA=" + std::to_string(request.amount);
            request.blockDetail = "CARD";
            if (!debt.collect(player, request)) {
                break;
            }
            for (std::size_t i = 0; i < players.size(); ++i) {
                if (players[i] != &player && !players[i]->isBankrupt()) {
                    players[i]->addMoney(perPlayerAmount);
                }
            }
            break;
        }
        case CardEffectType::RECEIVE_FROM_EACH_PLAYER: {
            const std::vector<Player*>& players = game.getPlayers();
            for (std::size_t i = 0; i < players.size(); ++i) {
                if (players[i] == &player || players[i]->isBankrupt()) {
                    continue;
                }
                PaymentRequest request;
                request.amount = value;
                request.payee = &player;
                request.allowDiscount = true;
                request.allowShield = true;
                request.actionName = sourceAction;
                request.detail = "BAYAR_KE_" + player.getName() + "=" + std::to_string(value);
                request.blockDetail = "CARD";
                debt.collect(*players[i], request);
            }
            break;
        }
    }
}

void CardEffectResolver::applyMoveToNearestRailroad(Player& player) {
    int bestDistance = INT_MAX;
    int targetPosition = player.getPosition();
    for (int position = 1; position <= game.board.size(); ++position) {
        Railroad* railroad = dynamic_cast<Railroad*>(game.board.getTile(position));
        if (railroad == 0) {
            continue;
        }
        int distance = position - player.getPosition();
        if (distance <= 0) {
            distance += game.board.size();
        }
        if (distance < bestDistance) {
            bestDistance = distance;
            targetPosition = position;
        }
    }
    game.movePlayerTo(player, targetPosition, true);
    game.resolveLanding(player, 0);
    game.logger.log(game.currentTurn, player.getName(), "KARTU", "STASIUN_TERDEKAT");
}

void CardEffectResolver::applyTeleport(Player& player) {
    if (player.isInJail()) {
        if (game.display != 0) {
            game.display->printMessage("TeleportCard tidak dapat memindahkan pemain yang sedang di penjara.");
        }
        return;
    }

    Tile* targetTile = 0;
    if (game.display != 0) {
        std::string input = game.display->getInput("Pilih kode atau posisi petak tujuan: ");
        if (!input.empty() && std::isdigit(static_cast<unsigned char>(input[0]))) {
            int position = std::atoi(input.c_str());
            if (position >= 1 && position <= game.board.size()) {
                targetTile = game.board.getTile(position);
            }
        } else {
            targetTile = findTileByCode(game.board, input);
        }
    }
    if (targetTile == 0) {
        targetTile = game.board.getTile(1);
    }

    game.movePlayerTo(player, targetTile->getPosition(), true);
    game.resolveLanding(player, 0);
    game.logger.log(game.currentTurn, player.getName(), "KARTU",
                    "Pakai TeleportCard -> pindah ke " + targetTile->getName() + " (" + targetTile->getCode() + ")");
}

void CardEffectResolver::applyLasso(Player& player) {
    const std::vector<Player*>& players = game.players;
    Player* target = 0;
    int bestDistance = INT_MAX;
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i] == &player || players[i]->isBankrupt() || players[i]->isInJail()) {
            continue;
        }
        int distance = players[i]->getPosition() - player.getPosition();
        if (distance <= 0) {
            distance += game.board.size();
        }
        if (distance < bestDistance) {
            bestDistance = distance;
            target = players[i];
        }
    }

    if (target == 0) {
        return;
    }

    if (target->hasActiveShield()) {
        game.logger.log(game.currentTurn, target->getName(), "SHIELD_BLOCK", "LASSO");
        return;
    }

    target->setPosition(player.getPosition());
    game.logger.log(game.currentTurn, player.getName(), "KARTU",
                    "Pakai LassoCard -> tarik " + target->getName());
    game.resolveLanding(*target, 0);
}

void CardEffectResolver::applyDemolition(Player& player) {
    std::vector<Property*> enemyProperties;
    const std::vector<Player*>& players = game.players;
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i] == &player || players[i]->isBankrupt()) {
            continue;
        }
        const std::vector<Property*>& owned = players[i]->getOwnedProperties();
        enemyProperties.insert(enemyProperties.end(), owned.begin(), owned.end());
    }
    if (enemyProperties.empty()) {
        return;
    }

    Property* targetProperty = enemyProperties[0];
    if (game.display != 0) {
        std::ostringstream list;
        list << "Pilih properti lawan untuk dihancurkan:\n";
        for (std::size_t i = 0; i < enemyProperties.size(); ++i) {
            list << i + 1 << ". " << enemyProperties[i]->getCode() << '\n';
        }
        game.display->printMessage(list.str());
        int choice = std::atoi(game.display->getInput("Pilihan: ").c_str());
        if (choice >= 1 && static_cast<std::size_t>(choice) <= enemyProperties.size()) {
            targetProperty = enemyProperties[static_cast<std::size_t>(choice - 1)];
        }
    }

    Player* owner = targetProperty->getOwner();
    if (owner != 0 && owner->hasActiveShield()) {
        game.logger.log(game.currentTurn, owner->getName(), "SHIELD_BLOCK", "DEMOLITION");
        return;
    }

    Street* street = dynamic_cast<Street*>(targetProperty);
    if (street != 0 && street->hasBuildings()) {
        street->resetBuildings();
    }
    game.logger.log(game.currentTurn, player.getName(), "KARTU",
                    "Pakai DemolitionCard -> target " + targetProperty->getCode());
}

void CardEffectResolver::applyChanceCard(Player& player, const ChanceCard& card) {
    applyCommonCardEffect(player, "KESEMPATAN", card.getEffectType(), card.getValue(), card.getTargetPosition());
}

void CardEffectResolver::applyCommunityChestCard(Player& player, const CommunityChestCard& card) {
    applyCommonCardEffect(player, "DANA_UMUM", card.getEffectType(), card.getValue(), card.getTargetPosition());
}

void CardEffectResolver::applySkillCard(Player& player, SkillCard& card) {
    const std::string& effect = card.getEffectCode();
    if (effect == "MoveCard") {
        if (player.isInJail()) {
            if (game.display != 0) {
                game.display->printMessage("MoveCard tidak dapat digunakan untuk bergerak saat di penjara.");
            }
            return;
        }
        game.movePlayer(player, card.getValue());
        game.resolveLanding(player, 0);
        game.logger.log(game.currentTurn, player.getName(), "KARTU",
                        "Pakai MoveCard -> maju " + std::to_string(card.getValue()) + " petak");
        return;
    }
    if (effect == "DiscountCard") {
        player.activateDiscount(card.getValue(), 1);
        if (game.display != 0) {
            game.display->printMessage("DiscountCard diaktifkan! Diskon " + std::to_string(card.getValue()) +
                                       "% berlaku selama giliran ini.");
        }
        return;
    }
    if (effect == "ShieldCard") {
        player.activateShield(1);
        if (game.display != 0) {
            game.display->printMessage("ShieldCard diaktifkan! Anda kebal terhadap tagihan atau sanksi selama giliran ini.");
        }
        return;
    }
    if (effect == "TeleportCard") {
        applyTeleport(player);
        return;
    }
    if (effect == "LassoCard") {
        applyLasso(player);
        return;
    }
    applyDemolition(player);
}
