#include "core/game_manager/GameManager.hpp"

#include <cctype>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "core/services/AuctionSession.hpp"
#include "core/services/DebtResolver.hpp"
#include "GameManagerInternal.hpp"
#include "models/base/Property.hpp"
#include "models/tiles/Street.hpp"
#include "models/base/Tile.hpp"

namespace {

std::string landingLabel(const Property& property) {
    return GameManagerInternal::prettyName(property.getName()) + " (" + property.getCode() + ")";
}

std::string propertyConditionLabel(const Property& property) {
    const Street* street = dynamic_cast<const Street*>(&property);
    if (street == 0) {
        return property.getPropertyType() == PropertyType::RAILROAD ? "Stasiun" : "Utilitas";
    }
    return GameManagerInternal::buildingLevelLabel(street->getBuildingLevel());
}

void appendWealthBreakdown(const Player& player, int& propertyValue, int& buildingValue) {
    propertyValue = 0;
    buildingValue = 0;
    const std::vector<Property*>& owned = player.getOwnedProperties();
    for (std::size_t i = 0; i < owned.size(); ++i) {
        if (owned[i] == 0) {
            continue;
        }

        propertyValue += owned[i]->getPurchasePrice() > 0 ? owned[i]->getPurchasePrice() : owned[i]->getAssetValue();
        const Street* street = dynamic_cast<const Street*>(owned[i]);
        if (street == 0) {
            continue;
        }

        const int level = street->getBuildingLevel();
        for (int built = 0; built < level; ++built) {
            buildingValue += (built < 4) ? street->getHouseCost() : street->getHotelCost();
        }
    }
}

}

std::pair<int, int> GameManager::rollMovementDice() {
    return useManualDice ? manualDiceValue : dice.rollPair();
}

void GameManager::setManualDice(int die1, int die2) {
    manualDiceValue = std::make_pair(die1, die2);
    useManualDice = true;
}

void GameManager::clearManualDice() { useManualDice = false; }

void GameManager::movePlayer(Player& player, int steps) {
    int oldPos = player.getPosition();
    player.move(steps, board.size());
    if (steps > 0 && player.getPosition() < oldPos) {
        bank.payPlayer(player, getGoSalary());
        logger.log(currentTurn, player.getName(), "PASS_GO", "Received GO salary");
    }
}

void GameManager::movePlayerTo(Player& player, int targetPosition, bool collectGoSalary) {
    int oldPos = player.getPosition();
    player.setPosition(targetPosition);
    if (collectGoSalary && targetPosition < oldPos) {
        bank.payPlayer(player, getGoSalary());
        logger.log(currentTurn, player.getName(), "PASS_GO", "Received GO salary");
    }
}

void GameManager::resolveLanding(Player& player, int diceTotal) {
    Tile* tile = board.getTile(player.getPosition());
    if (tile == 0) throw std::runtime_error("Player landed on invalid tile.");
    logger.log(currentTurn, player.getName(), "LAND", tile->getCode());
    tile->onLanded(*this, player, diceTotal);
}

void GameManager::handlePropertyLanding(Player& player, Property& property, int diceTotal) {
    if (display != 0) {
        display->printMessage("Kamu mendarat di " + landingLabel(property) + "!");
    }

    if (property.getStatus() == PropertyStatus::BANK) {
        if (property.getPropertyType() == PropertyType::STREET) {
            if (display != 0) {
                commandPrintDeed(property.getCode());
                display->printMessage("Uang kamu saat ini: " + GameManagerInternal::formatMoney(player.getMoney()));
                int shownPrice = GameManagerInternal::applyDiscount(player, property.getPurchasePrice());
                std::string ans = GameManagerInternal::toUpper(
                    display->getInput("Apakah kamu ingin membeli properti ini seharga "
                        + GameManagerInternal::formatMoney(shownPrice) + "? (y/n): "));
                if (ans == "Y" || ans == "YES") {
                    handleStreetPurchase(player, dynamic_cast<Street&>(property));
                } else {
                    display->printMessage("Properti ini akan masuk ke sistem lelang...");
                    handleAuction(property);
                }
            } else {
                handleStreetPurchase(player, dynamic_cast<Street&>(property));
            }
        } else {
            property.setOwner(&player);
            player.addProperty(&property);
            logger.log(currentTurn, player.getName(), "AUTO_CLAIM", property.getCode());
            if (display != 0) {
                display->printMessage("Belum ada yang menginjaknya duluan, "
                    + GameManagerInternal::prettyName(property.getName()) + " kini menjadi milikmu!");
            }
        }
    } else if (property.getOwner() != &player) {
        if (property.isMortgaged()) {
            if (display != 0 && property.getOwner() != 0) {
                display->printMessage("Kamu mendarat di " + landingLabel(property)
                    + ", milik " + property.getOwner()->getName() + ".");
                display->printMessage("Properti ini sedang digadaikan [M]. Tidak ada sewa yang dikenakan.");
            }
            return;
        }
        handleRentPayment(player, property, diceTotal);
    }
}

void GameManager::handleStreetPurchase(Player& player, Street& street) {
    int price = GameManagerInternal::applyDiscount(player, street.getPurchasePrice());
    if (!bank.canPlayerAfford(player, price)) {
        if (display != 0) {
            display->printMessage("Uang kamu saat ini: " + GameManagerInternal::formatMoney(player.getMoney()));
            display->printMessage("Properti ini akan masuk ke sistem lelang...");
        }
        handleAuction(street);
        return;
    }
    bank.collectFromPlayer(player, price);
    street.setOwner(&player);
    street.setStatus(PropertyStatus::OWNED);
    player.addProperty(&street);
    logger.log(currentTurn, player.getName(), "BUY", street.getCode());
    if (display != 0) {
        display->printMessage(GameManagerInternal::prettyName(street.getName()) + " kini menjadi milikmu!");
        display->printMessage("Uang tersisa: " + GameManagerInternal::formatMoney(player.getMoney()));
    }
}

void GameManager::handleRentPayment(Player& visitor, Property& property, int diceTotal) {
    Player* owner = property.getOwner();
    if (owner == 0) return;

    DebtResolver debt(*this);
    const int rent = property.getCurrentRent(diceTotal);
    const int finalRent = debt.adjustedAmount(visitor, rent, true);
    const int visitorBefore = visitor.getMoney();
    const int ownerBefore = owner->getMoney();

    if (display != 0) {
        display->printMessage("Kamu mendarat di " + landingLabel(property) + ", milik " + owner->getName() + "!");
        display->printMessage("Kondisi      : " + propertyConditionLabel(property));
        display->printMessage("Sewa         : " + GameManagerInternal::formatMoney(finalRent));
        const std::string festivalInfo = GameManagerInternal::festivalStatusLabel(property);
        if (!festivalInfo.empty()) {
            display->printMessage("Festival     : " + festivalInfo);
        }
        if (visitorBefore < finalRent) {
            display->printMessage("Kamu tidak mampu membayar sewa penuh! (" + GameManagerInternal::formatMoney(finalRent) + ")");
            display->printMessage("Uang kamu saat ini: " + GameManagerInternal::formatMoney(visitorBefore));
        }
    }

    PaymentRequest request;
    request.amount = rent;
    request.payee = owner;
    request.creditor = owner;
    request.allowDiscount = true;
    request.allowShield = true;
    request.actionName = "PAY_RENT";
    request.detail = property.getCode() + "=" + std::to_string(finalRent);
    request.blockDetail = property.getCode();
    if (debt.collect(visitor, request) && display != 0) {
        display->printMessage("Uang kamu     : " + GameManagerInternal::formatMoney(visitorBefore)
            + " -> " + GameManagerInternal::formatMoney(visitor.getMoney()));
        display->printMessage("Uang " + owner->getName() + " : " + GameManagerInternal::formatMoney(ownerBefore)
            + " -> " + GameManagerInternal::formatMoney(owner->getMoney()));
    }
}

void GameManager::handleAuction(Property& property) {
    AuctionSession(*this).run(property, static_cast<std::size_t>(currentPlayerIndex + 1));
}

void GameManager::handleFestival(Player& player) {
    const std::vector<Property*>& props = player.getOwnedProperties();
    if (display != 0) {
        display->printMessage("Kamu mendarat di petak Festival!");
    }
    if (props.empty()) {
        if (display != 0) {
            display->printMessage("Kamu belum memiliki properti untuk diberi efek festival.");
        }
        return;
    }

    Property* target = props[0];
    if (display != 0) {
        std::ostringstream oss;
        oss << "\nDaftar properti milikmu:\n";
        for (std::size_t i = 0; i < props.size(); ++i) {
            oss << "- " << props[i]->getCode() << " (" << GameManagerInternal::prettyName(props[i]->getName()) << ")\n";
        }
        display->printMessage(oss.str());

        while (true) {
            std::string input = display->getInput("Masukkan kode properti: ");
            if (input.empty()) {
                target = props[0];
                break;
            }

            Property* chosen = 0;
            if (std::isdigit(static_cast<unsigned char>(input[0]))) {
                int index = std::atoi(input.c_str());
                if (index >= 1 && static_cast<std::size_t>(index) <= props.size()) {
                    chosen = props[static_cast<std::size_t>(index - 1)];
                }
            } else {
                chosen = board.getPropertyByCode(GameManagerInternal::toUpper(input));
            }
            if (chosen == 0) {
                display->printMessage("-> Kode properti tidak valid!");
                continue;
            }
            if (chosen->getOwner() != &player) {
                display->printMessage("-> Properti bukan milikmu!");
                continue;
            }
            target = chosen;
            break;
        }
    }

    const int previousMultiplier = target->getFestivalMultiplier();
    const int previousRent = target->getCurrentRent(7);
    target->applyFestivalBoost();
    logger.log(currentTurn, player.getName(), "FESTIVAL", target->getCode());
    if (display != 0) {
        const int currentRent = target->getCurrentRent(7);
        display->printMessage("");
        if (previousMultiplier <= 1) {
            display->printMessage("Efek festival aktif!");
            display->printMessage("Sewa awal: " + GameManagerInternal::formatMoney(previousRent));
            display->printMessage("Sewa sekarang: " + GameManagerInternal::formatMoney(currentRent));
            display->printMessage("Durasi: 3 giliran");
        } else if (previousMultiplier < 8) {
            display->printMessage("Efek diperkuat!");
            display->printMessage("Sewa sebelumnya: " + GameManagerInternal::formatMoney(previousRent));
            display->printMessage("Sewa sekarang: " + GameManagerInternal::formatMoney(currentRent));
            display->printMessage("Durasi di-reset menjadi: 3 giliran");
        } else {
            display->printMessage("Efek sudah maksimum (harga sewa sudah digandakan tiga kali)");
            display->printMessage("Durasi di-reset menjadi: 3 giliran");
        }
        if (target->getPropertyType() == PropertyType::UTILITY) {
            display->printMessage("Catatan: Untuk utilitas, festival tetap menggandakan hasil sewa saat utilitas dipakai.");
        }
    }
}

void GameManager::handleTax(Player& player, ActionType taxType) {
    DebtResolver debt(*this);
    int amount = 0;
    if (taxType == ActionType::TAX_PPH) {
        const int flat = gameConfig.tax.pphFlat;
        const int percent = (player.getTotalWealth() * gameConfig.tax.pphPercentage) / 100;
        if (display != 0) {
            display->printMessage("Kamu mendarat di Pajak Penghasilan (PPH)!");
            display->printMessage("Pilih opsi pembayaran pajak:");
            display->printMessage("1. Bayar flat " + GameManagerInternal::formatMoney(flat));
            display->printMessage("2. Bayar " + std::to_string(gameConfig.tax.pphPercentage) + "% dari total kekayaan");
            display->printMessage("(Pilih sebelum menghitung kekayaan!)");
            std::string choice = display->getInput("Pilihan (1/2): ");
            amount = (choice == "2") ? percent : flat;
            if (choice == "2") {
                int propertyValue = 0;
                int buildingValue = 0;
                appendWealthBreakdown(player, propertyValue, buildingValue);
                display->printMessage("");
                display->printMessage("Total kekayaan kamu:");
                display->printMessage("- Uang tunai          : " + GameManagerInternal::formatMoney(player.getMoney()));
                display->printMessage("- Harga beli properti : " + GameManagerInternal::formatMoney(propertyValue));
                display->printMessage("- Harga beli bangunan : " + GameManagerInternal::formatMoney(buildingValue));
                display->printMessage("Total                 : " + GameManagerInternal::formatMoney(player.getMoney() + propertyValue + buildingValue));
                display->printMessage("Pajak " + std::to_string(gameConfig.tax.pphPercentage) + "%             : " + GameManagerInternal::formatMoney(percent));
            }
        } else {
            amount = std::min(flat, percent);
        }
    } else {
        if (display != 0) {
            display->printMessage("Kamu mendarat di Pajak Barang Mewah (PBM)!");
            display->printMessage("Pajak sebesar " + GameManagerInternal::formatMoney(gameConfig.tax.pbmFlat) + " langsung dipotong.");
        }
        amount = gameConfig.tax.pbmFlat;
    }

    const int playerBefore = player.getMoney();
    const int finalAmount = debt.adjustedAmount(player, amount, true);
    if (display != 0 && playerBefore < finalAmount) {
        if (taxType == ActionType::TAX_PPH) {
            display->printMessage("Kamu tidak mampu membayar pajak " + GameManagerInternal::formatMoney(finalAmount) + "!");
        } else {
            display->printMessage("Kamu tidak mampu membayar pajak!");
        }
        display->printMessage("Uang kamu saat ini: " + GameManagerInternal::formatMoney(playerBefore));
    }

    PaymentRequest request;
    request.amount = amount;
    request.allowDiscount = true;
    request.allowShield = true;
    request.actionName = "PAY_TAX";
    request.detail = std::to_string(finalAmount);
    request.blockDetail = "TAX";
    if (debt.collect(player, request) && display != 0) {
        display->printMessage("Uang kamu: " + GameManagerInternal::formatMoney(playerBefore)
            + " -> " + GameManagerInternal::formatMoney(player.getMoney()));
    }
}

void GameManager::handleJail(Player& player) {
    if (player.hasActiveShield()) {
        logger.log(currentTurn, player.getName(), "SHIELD_BLOCK", "JAIL");
        if (display != 0 && &player == &getCurrentPlayer()) {
            display->printMessage("[SHIELD ACTIVE]: Efek ShieldCard melindungi Anda!");
        }
        return;
    }
    player.sendToJail(11);
    logger.log(currentTurn, player.getName(), "JAIL", "Sent to jail");
}

void GameManager::handleBankruptcy(Player& player, Player* creditor) {
    DebtResolver(*this).declareBankruptcy(player, creditor);
}

void GameManager::drawChanceCard(Player& player) {
    if (display != 0) {
        display->printMessage("Kamu mendarat di Petak Kesempatan!");
        display->printMessage("Mengambil kartu...");
    }
    ChanceCard* card = chanceDeck.draw();
    logger.log(currentTurn, player.getName(), "DRAW_CHANCE", card->getName());
    if (display != 0) {
        display->printMessage("Kartu: \"" + card->getDescription() + "\"");
    }
    card->applyEffect(*this, player);
    chanceDeck.putBack(card);
    chanceDeck.shuffle();
}

void GameManager::drawCommunityChestCard(Player& player) {
    if (display != 0) {
        display->printMessage("Kamu mendarat di Petak Dana Umum!");
        display->printMessage("Mengambil kartu...");
    }
    CommunityChestCard* card = communityDeck.draw();
    logger.log(currentTurn, player.getName(), "DRAW_COMMUNITY", card->getName());
    if (display != 0) {
        display->printMessage("Kartu: \"" + card->getDescription() + "\"");
    }
    card->applyEffect(*this, player);
    communityDeck.putBack(card);
    communityDeck.shuffle();
}

SkillCard* GameManager::drawSkillCard() {
    if (skillDeck.empty()) rebuildSkillDeckFromDiscard();
    if (skillDeck.empty()) return 0;
    return skillDeck.draw();
}

void GameManager::awardSkillCard(Player& player) {
    SkillCard* card = drawSkillCard();
    if (card == 0) return;
    if (player.getInventory().isSkillCardFull()) {
        handleAutomaticSkillDrop(player);
    }
    player.getInventory().addSkillCard(card);
    logger.log(currentTurn, player.getName(), "DRAW_SKILL", card->getName());
    if (display != 0 && &player == &getCurrentPlayer()) {
        display->printMessage("Kartu yang didapat: " + card->getName() + ".");
    }
}

void GameManager::discardSkillCard(SkillCard* skillCard) {
    if (skillCard != 0) discardedSkillCards.push_back(skillCard);
}

void GameManager::rebuildSkillDeckFromDiscard() {
    for (std::size_t i = 0; i < discardedSkillCards.size(); ++i) {
        skillDeck.addCard(discardedSkillCards[i]);
    }
    discardedSkillCards.clear();
    skillDeck.shuffle();
}

void GameManager::tickPlayerSkillDurations(Player& player) {
    player.tickActiveEffects();
}

int GameManager::getGoSalary() const { return gameConfig.special.goSalary; }
int GameManager::getJailFine() const { return gameConfig.special.jailFine; }
int GameManager::getMaxTurn() const { return gameConfig.misc.maxTurn; }

bool GameManager::isGameOver() const {
    int active = 0;
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (!players[i]->isBankrupt()) ++active;
    }
    if (active <= 1) return true;
    if (getMaxTurn() > 0 && currentTurn > getMaxTurn()) return true;
    return false;
}
