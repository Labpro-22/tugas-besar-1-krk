#include "core/services/DebtResolver.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "core/game_manager/GameManager.hpp"
#include "../game_manager/GameManagerInternal.hpp"
#include "models/player/Player.hpp"
#include "models/base/Property.hpp"
#include "models/cards/SkillCard.hpp"
#include "models/tiles/Street.hpp"

namespace {

std::string paymentFailureMessage(const PaymentRequest& request, Player* creditor, int amount) {
    if (request.actionName == "PAY_RENT") {
        return "Kamu tidak dapat membayar sewa " + GameManagerInternal::formatMoney(amount)
            + " kepada " + (creditor != 0 ? creditor->getName() : "Bank") + "!";
    }
    if (request.actionName == "PAY_TAX") {
        return "Kamu tidak dapat membayar pajak " + GameManagerInternal::formatMoney(amount) + "!";
    }
    if (request.actionName == "PAY_JAIL_FINE") {
        return "Kamu tidak dapat membayar denda penjara " + GameManagerInternal::formatMoney(amount) + "!";
    }
    return "Kamu tidak dapat membayar kewajiban " + GameManagerInternal::formatMoney(amount) + "!";
}

std::string propertySummary(const Property& property) {
    std::string summary = GameManagerInternal::prettyName(property.getName())
        + " (" + property.getCode() + ")"
        + "  [" + GameManagerInternal::propertyGroupLabel(property) + "]  ";

    if (property.isMortgaged()) {
        summary += "MORTGAGED [M]";
        return summary;
    }

    const Street* street = dynamic_cast<const Street*>(&property);
    summary += "OWNED";
    if (street != 0 && street->getBuildingLevel() > 0) {
        summary += " (" + GameManagerInternal::buildingLevelLabel(street->getBuildingLevel()) + ")";
    }
    return summary;
}

struct LiquidationOption {
    std::string action;
    Property* property;
};

}

PaymentRequest::PaymentRequest()
    : amount(0),
      payee(0),
      creditor(0),
      allowDiscount(false),
      allowShield(false),
      allowLiquidation(true) {}

DebtResolver::DebtResolver(GameManager& game) : game(game) {}

int DebtResolver::adjustedAmount(const Player& player, int amount, bool allowDiscount) const {
    if (!allowDiscount) {
        return amount;
    }
    return GameManagerInternal::applyDiscount(player, amount);
}

int DebtResolver::bankSaleValue(const Property& property) const {
    if (property.isMortgaged()) {
        return 0;
    }

    int saleValue = property.getPurchasePrice();
    if (saleValue <= 0) {
        saleValue = property.getMortgageValue();
    }

    const Street* street = dynamic_cast<const Street*>(&property);
    if (street != 0) {
        for (int level = 0; level < street->getBuildingLevel(); ++level) {
            const int buildCost = (level < 4) ? street->getHouseCost() : street->getHotelCost();
            saleValue += buildCost / 2;
        }
    }

    return saleValue;
}

int DebtResolver::maxLiquidationValue(const Player& player) const {
    int total = 0;
    const std::vector<Property*>& properties = player.getOwnedProperties();
    for (std::size_t i = 0; i < properties.size(); ++i) {
        if (properties[i] != 0) {
            total += bankSaleValue(*properties[i]);
        }
    }
    return total;
}

bool DebtResolver::sellPropertyToBank(Player& player, Property& property) {
    if (property.getOwner() != &player || property.isMortgaged()) {
        return false;
    }

    int saleValue = bankSaleValue(property);
    Street* street = dynamic_cast<Street*>(&property);
    if (street != 0) {
        street->resetBuildings();
    }

    player.removeProperty(&property);
    property.clearOwner();
    property.setStatus(PropertyStatus::BANK);
    game.bank.payPlayer(player, saleValue);
    game.logger.log(game.currentTurn, player.getName(), "LIQUIDATE_SELL",
                    property.getCode() + "=" + std::to_string(saleValue));
    return true;
}

bool DebtResolver::mortgageProperty(Player& player, Property& property) {
    if (property.getOwner() != &player || property.isMortgaged()) {
        return false;
    }

    Street* street = dynamic_cast<Street*>(&property);
    if (street != 0) {
        const std::vector<Property*> group = game.getBoard().getPropertiesByColorGroup(street->getColorGroup());
        for (std::size_t i = 0; i < group.size(); ++i) {
            Street* groupStreet = dynamic_cast<Street*>(group[i]);
            while (groupStreet != 0 && groupStreet->hasBuildings()) {
                const int saleValue = (groupStreet->getBuildingLevel() >= 5 ? groupStreet->getHotelCost()
                                                                            : groupStreet->getHouseCost()) / 2;
                groupStreet->sellOneBuilding();
                game.bank.payPlayer(player, saleValue);
                game.logger.log(game.currentTurn, player.getName(), "LIQUIDATE_BUILDING",
                                groupStreet->getCode() + "=" + std::to_string(saleValue));
            }
        }
    }

    if (property.hasBuildings()) {
        return false;
    }

    property.mortgage();
    game.bank.payPlayer(player, property.getMortgageValue());
    game.logger.log(game.currentTurn, player.getName(), "LIQUIDATE_MORTGAGE",
                    property.getCode() + "=" + std::to_string(property.getMortgageValue()));
    return true;
}

bool DebtResolver::autoLiquidate(Player& player, int requiredAmount) {
    while (player.getMoney() < requiredAmount) {
        Property* bestProperty = 0;
        int bestValue = -1;
        const std::vector<Property*>& properties = player.getOwnedProperties();
        for (std::size_t i = 0; i < properties.size(); ++i) {
            const int value = bankSaleValue(*properties[i]);
            if (value > bestValue) {
                bestValue = value;
                bestProperty = properties[i];
            }
        }
        if (bestProperty == 0 || bestValue <= 0) {
            return false;
        }
        if (!sellPropertyToBank(player, *bestProperty)) {
            return false;
        }
    }
    return true;
}

bool DebtResolver::interactiveLiquidate(Player& player, int requiredAmount) {
    while (player.getMoney() < requiredAmount) {
        if (game.display == 0) {
            return autoLiquidate(player, requiredAmount);
        }

        std::vector<LiquidationOption> options;
        std::ostringstream panel;
        panel << "=== Panel Likuidasi ===\n";
        panel << "Uang kamu saat ini: " << GameManagerInternal::formatMoney(player.getMoney())
              << "  |  Kewajiban: " << GameManagerInternal::formatMoney(requiredAmount) << "\n\n";
        panel << "[Jual ke Bank]\n";
        for (std::size_t i = 0; i < player.getOwnedProperties().size(); ++i) {
            Property* property = player.getOwnedProperties()[i];
            if (property == 0 || property->isMortgaged()) {
                continue;
            }
            const int value = bankSaleValue(*property);
            if (value <= 0) {
                continue;
            }
            options.push_back(LiquidationOption{"JUAL", property});
            panel << options.size() << ". "
                  << GameManagerInternal::prettyName(property->getName())
                  << " (" << property->getCode() << ")"
                  << "  [" << GameManagerInternal::propertyGroupLabel(*property) << "]  "
                  << "Harga Jual: " << GameManagerInternal::formatMoney(value) << '\n';
        }

        panel << "\n[Gadaikan]\n";
        for (std::size_t i = 0; i < player.getOwnedProperties().size(); ++i) {
            Property* property = player.getOwnedProperties()[i];
            if (property == 0 || property->isMortgaged()) {
                continue;
            }
            options.push_back(LiquidationOption{"GADAI", property});
            panel << options.size() << ". "
                  << GameManagerInternal::prettyName(property->getName())
                  << " (" << property->getCode() << ")"
                  << "  [" << GameManagerInternal::propertyGroupLabel(*property) << "]  "
                  << "Nilai Gadai: " << GameManagerInternal::formatMoney(property->getMortgageValue()) << '\n';
        }
        game.display->printMessage(panel.str());

        std::string input = game.display->getInput("Pilih aksi (0 jika sudah cukup): ");
        if (input.empty()) {
            return autoLiquidate(player, requiredAmount);
        }
        std::stringstream ss(input);
        std::string command;
        std::string code;
        ss >> command >> code;
        command = GameManagerInternal::toUpper(command);
        Property* property = 0;
        if (!command.empty() && std::isdigit(static_cast<unsigned char>(command[0]))) {
            int choice = std::atoi(command.c_str());
            if (choice == 0) {
                if (player.getMoney() >= requiredAmount) {
                    return true;
                }
                game.display->printMessage("Likuidasi belum cukup untuk menutup kewajiban.");
                continue;
            }
            if (choice < 1 || static_cast<std::size_t>(choice) > options.size()) {
                game.display->printMessage("Pilihan likuidasi tidak valid.");
                continue;
            }
            command = options[static_cast<std::size_t>(choice - 1)].action;
            property = options[static_cast<std::size_t>(choice - 1)].property;
        } else {
            property = code.empty() ? 0 : game.board.getPropertyByCode(code);
        }
        if (property == 0 || property->getOwner() != &player) {
            game.display->printMessage("Properti tidak valid untuk likuidasi.");
            continue;
        }

        bool success = false;
        if (command == "JUAL") {
            success = sellPropertyToBank(player, *property);
        } else if (command == "GADAI") {
            success = mortgageProperty(player, *property);
        } else {
            game.display->printMessage("Perintah likuidasi tidak dikenal.");
            continue;
        }

        if (!success) {
            game.display->printMessage("Aksi likuidasi tersebut tidak dapat dilakukan.");
        } else {
            game.display->printMessage("Uang kamu sekarang: " + GameManagerInternal::formatMoney(player.getMoney()));
        }
    }
    return true;
}

bool DebtResolver::liquidate(Player& player, int requiredAmount, Player* creditor) {
    if (player.getMoney() >= requiredAmount) {
        return true;
    }

    if (player.getMoney() + maxLiquidationValue(player) < requiredAmount) {
        return false;
    }

    if (game.display != 0) {
        game.display->printMessage("");
        game.display->printMessage("Uang kamu       : " + GameManagerInternal::formatMoney(player.getMoney()));
        game.display->printMessage("Total kewajiban : " + GameManagerInternal::formatMoney(requiredAmount));
        game.display->printMessage("Kekurangan      : " + GameManagerInternal::formatMoney(requiredAmount - player.getMoney()));
        game.display->printMessage("");
        game.display->printMessage("Estimasi dana maksimum dari likuidasi:");
        game.display->printMessage("  Total potensi        -> " + GameManagerInternal::formatMoney(maxLiquidationValue(player)));
        game.display->printMessage("Dana likuidasi dapat menutup kewajiban.");
        game.display->printMessage("Kamu wajib melikuidasi aset untuk membayar.");
        game.display->printMessage("");
    }

    const bool success = interactiveLiquidate(player, requiredAmount);
    if (!success && creditor != 0) {
        game.logger.log(game.currentTurn, player.getName(), "FAILED_LIQUIDATION", creditor->getName());
    }
    return success;
}

bool DebtResolver::collect(Player& payer, const PaymentRequest& request) {
    const int finalAmount = adjustedAmount(payer, request.amount, request.allowDiscount);
    if (finalAmount <= 0) {
        return true;
    }

    if (request.allowShield && payer.hasActiveShield()) {
        game.logger.log(game.currentTurn, payer.getName(), "SHIELD_BLOCK",
                        request.blockDetail.empty() ? request.detail : request.blockDetail);
        if (game.display != 0) {
            game.display->printMessage("[SHIELD ACTIVE]: Efek ShieldCard melindungi Anda!");
            game.display->printMessage("Tagihan M" + std::to_string(finalAmount) +
                                       " dibatalkan. Uang Anda tetap: M" +
                                       std::to_string(payer.getMoney()) + ".");
        }
        return true;
    }

    if (payer.getMoney() < finalAmount && request.allowLiquidation) {
        if (game.display != 0) {
            game.display->printMessage(paymentFailureMessage(request, request.creditor, finalAmount));
            game.display->printMessage("");
        }
        if (!liquidate(payer, finalAmount, request.creditor)) {
            declareBankruptcy(payer, request.creditor);
            return false;
        }
    }

    if (payer.getMoney() < finalAmount) {
        declareBankruptcy(payer, request.creditor);
        return false;
    }

    payer.deductMoney(finalAmount);
    if (request.payee != 0) {
        request.payee->addMoney(finalAmount);
    }

    if (!request.actionName.empty()) {
        game.logger.log(game.currentTurn, payer.getName(), request.actionName,
                        request.detail.empty() ? std::to_string(finalAmount) : request.detail);
    }
    return true;
}

void DebtResolver::declareBankruptcy(Player& player, Player* creditor) {
    if (player.isBankrupt()) {
        return;
    }

    if (game.display != 0) {
        game.display->printMessage(player.getName() + " dinyatakan BANGKRUT!");
        game.display->printMessage("Kreditor: " + std::string(creditor != 0 ? creditor->getName() : "Bank"));
        game.display->printMessage("");
    }

    game.logger.log(game.currentTurn, player.getName(), "BANKRUPTCY",
                    creditor != 0 ? creditor->getName() : "BANK");

    std::vector<Property*> owned = player.getOwnedProperties();
    if (game.display != 0 && creditor != 0) {
        game.display->printMessage("Pengalihan aset ke " + creditor->getName() + ":");
        if (player.getMoney() > 0) {
            game.display->printMessage("  - Uang tunai sisa  : " + GameManagerInternal::formatMoney(player.getMoney()));
        }
        for (std::size_t i = 0; i < owned.size(); ++i) {
            game.display->printMessage("  - " + propertySummary(*owned[i]));
        }
        game.display->printMessage("");
    }
    for (std::size_t i = 0; i < owned.size(); ++i) {
        Property* property = owned[i];
        Street* street = dynamic_cast<Street*>(property);
        player.removeProperty(property);

        if (creditor != 0) {
            property->setOwner(creditor);
            creditor->addProperty(property);
            property->setStatus(property->isMortgaged() ? PropertyStatus::MORTGAGED : PropertyStatus::OWNED);
        } else {
            if (street != 0) {
                street->resetBuildings();
            }
            property->clearOwner();
            property->setStatus(PropertyStatus::BANK);
        }
    }

    if (creditor == 0) {
        if (game.display != 0) {
            game.display->printMessage("Uang sisa " + GameManagerInternal::formatMoney(player.getMoney()) + " diserahkan ke Bank.");
            game.display->printMessage("Seluruh properti dikembalikan ke status BANK.");
            game.display->printMessage("Bangunan dihancurkan - stok dikembalikan ke Bank.");
            if (!owned.empty()) {
                game.display->printMessage("");
                game.display->printMessage("Properti akan dilelang satu per satu:");
                for (std::size_t i = 0; i < owned.size(); ++i) {
                    game.display->printMessage("  -> Lelang: "
                        + GameManagerInternal::prettyName(owned[i]->getName()) + " (" + owned[i]->getCode() + ") ...");
                }
            }
        }
        for (std::size_t i = 0; i < owned.size(); ++i) {
            game.handleAuction(*owned[i]);
        }
    } else if (player.getMoney() > 0) {
        creditor->addMoney(player.getMoney());
        if (game.display != 0) {
            game.display->printMessage(creditor->getName() + " menerima semua aset " + player.getName() + ".");
        }
    }

    while (player.getInventory().getSkillCardCount() > 0) {
        SkillCard* card = player.getInventory().takeSkillCard(0);
        game.discardSkillCard(card);
    }

    player.addMoney(-player.getMoney());
    player.clearActiveEffects();
    player.setBankrupt(true);

    if (game.display != 0) {
        game.display->printMessage(player.getName() + " telah keluar dari permainan.");
    }
}
