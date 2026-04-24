#include "core/services/AuctionSession.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/game_manager/GameManager.hpp"
#include "../game_manager/GameManagerInternal.hpp"
#include "models/player/Player.hpp"
#include "models/base/Property.hpp"

namespace {

std::size_t nextEligiblePlayerIndex(const std::vector<Player*>& players, std::size_t start) {
    if (players.empty()) {
        throw std::runtime_error("Auction cannot start without players.");
    }

    std::size_t index = start % players.size();
    for (std::size_t checked = 0; checked < players.size(); ++checked) {
        if (!players[index]->isBankrupt()) {
            return index;
        }
        index = (index + 1) % players.size();
    }

    return start % players.size();
}

int openingBidFor(const Property& /*property*/) {
    return 1;
}

}

AuctionSession::AuctionSession(GameManager& game) : game(game) {}

bool AuctionSession::run(Property& property, std::size_t startingPlayerIndex) {
    const std::vector<Player*>& players = game.players;
    int activeCount = 0;
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (!players[i]->isBankrupt()) {
            ++activeCount;
        }
    }
    if (activeCount <= 0) {
        return false;
    }

    const int openingBid = openingBidFor(property);
    std::size_t cursor = nextEligiblePlayerIndex(players, startingPlayerIndex);
    Player* highestBidder = 0;
    int highestBid = 0;
    int passCountSinceBid = 0;
    bool hasAnyBid = false;
    bool anyAffordable = false;
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (!players[i]->isBankrupt() && players[i]->canAfford(openingBid)) {
            anyAffordable = true;
            break;
        }
    }
    if (!anyAffordable) {
        return false;
    }

    if (game.display == 0) {
        for (int checked = 0; checked < static_cast<int>(players.size()); ++checked) {
            Player* bidder = players[cursor];
            if (!bidder->isBankrupt() && bidder->canAfford(openingBid)) {
                bidder->deductMoney(openingBid);
                property.setOwner(bidder);
                property.setStatus(PropertyStatus::OWNED);
                bidder->addProperty(&property);
                game.logger.log(game.currentTurn, bidder->getName(), "AUCTION_WIN",
                                property.getCode() + "=" + std::to_string(openingBid));
                return true;
            }
            cursor = (cursor + 1) % players.size();
        }
        return false;
    }

    game.display->printMessage("Properti " + GameManagerInternal::prettyName(property.getName())
        + " (" + property.getCode() + ") akan dilelang!");
    if (!players.empty()) {
        game.display->printMessage("");
        game.display->printMessage("Urutan lelang dimulai dari pemain setelah "
            + players[(startingPlayerIndex + players.size() - 1) % players.size()]->getName() + ".");
    }
    game.display->printMessage("");

    while (true) {
        if (hasAnyBid && passCountSinceBid >= activeCount - 1) {
            break;
        }

        cursor = nextEligiblePlayerIndex(players, cursor);
        Player* bidder = players[cursor];
        const bool forceBid = !hasAnyBid && passCountSinceBid >= activeCount - 1;
        const int minimumBid = hasAnyBid ? highestBid + 1 : openingBid;

        game.display->printMessage("Giliran: " + bidder->getName());
        game.display->printMessage("Aksi (" + std::string(forceBid ? "BID <jumlah>" : "PASS / BID <jumlah>") + ")");
        std::string input = game.display->getInput("> ");
        if (input.empty()) {
            if (forceBid) {
                input = "BID " + std::to_string(minimumBid);
            } else {
                input = "PASS";
            }
        }
        std::stringstream ss(input);
        std::string command;
        ss >> command;
        command = GameManagerInternal::toUpper(command);

        if (command == "PASS" && !forceBid) {
            ++passCountSinceBid;
            game.logger.log(game.currentTurn, bidder->getName(), "AUCTION_PASS", property.getCode());
            cursor = (cursor + 1) % players.size();
            continue;
        }

        if (command == "BID") {
            int bidAmount = 0;
            ss >> bidAmount;
            if (bidAmount < minimumBid) {
                game.display->printMessage("Bid terlalu kecil.");
                continue;
            }
            if (!bidder->canAfford(bidAmount)) {
                game.display->printMessage("Uang pemain tidak cukup untuk bid tersebut.");
                continue;
            }

            highestBid = bidAmount;
            highestBidder = bidder;
            hasAnyBid = true;
            passCountSinceBid = 0;
            game.display->printMessage("Penawaran tertinggi: "
                + GameManagerInternal::formatMoney(bidAmount) + " (" + bidder->getName() + ")");
            game.logger.log(game.currentTurn, bidder->getName(), "AUCTION_BID",
                            property.getCode() + "=" + std::to_string(bidAmount));
            cursor = (cursor + 1) % players.size();
            continue;
        }

        game.display->printMessage(forceBid ? "Pemain ini wajib melakukan BID." : "Masukan tidak valid.");
    }

    if (highestBidder == 0) {
        return false;
    }

    highestBidder->deductMoney(highestBid);
    property.setOwner(highestBidder);
    property.setStatus(PropertyStatus::OWNED);
    highestBidder->addProperty(&property);
    game.display->printMessage("");
    game.display->printMessage("Lelang selesai!");
    game.display->printMessage("Pemenang: " + highestBidder->getName());
    game.display->printMessage("Harga akhir: " + GameManagerInternal::formatMoney(highestBid));
    game.display->printMessage("");
    game.display->printMessage("Properti " + GameManagerInternal::prettyName(property.getName())
        + " (" + property.getCode() + ") kini dimiliki " + highestBidder->getName() + ".");
    game.logger.log(game.currentTurn, highestBidder->getName(), "AUCTION_WIN",
                    property.getCode() + "=" + std::to_string(highestBid));
    return true;
}
