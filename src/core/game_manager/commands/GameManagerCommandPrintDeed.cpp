#include "core/game_manager/GameManager.hpp"

#include <array>
#include <map>
#include <sstream>
#include <stdexcept>

#include "../GameManagerInternal.hpp"
#include "models/base/Property.hpp"
#include "models/tiles/Railroad.hpp"
#include "models/tiles/Street.hpp"
#include "models/tiles/Utility.hpp"

namespace {

const int DEED_WIDTH = 32;

std::string padRight(const std::string& text, int width) {
    if (static_cast<int>(text.size()) >= width) {
        return text.substr(0, static_cast<std::size_t>(width));
    }
    return text + std::string(static_cast<std::size_t>(width - static_cast<int>(text.size())), ' ');
}

std::string centerText(const std::string& text, int width) {
    if (static_cast<int>(text.size()) >= width) {
        return text.substr(0, static_cast<std::size_t>(width));
    }

    int left = (width - static_cast<int>(text.size())) / 2;
    int right = width - static_cast<int>(text.size()) - left;
    return std::string(static_cast<std::size_t>(left), ' ')
         + text
         + std::string(static_cast<std::size_t>(right), ' ');
}

void appendDeedLine(std::ostringstream& oss, const std::string& text) {
    oss << "| " << padRight(text, DEED_WIDTH) << " |\n";
}

void appendLabelValue(std::ostringstream& oss, const std::string& label, const std::string& value) {
    appendDeedLine(oss, padRight(label, 18) + ": " + value);
}

}

void GameManager::commandPrintDeed(const std::string& propertyCode) const {
    if (display == 0) return;

    std::string code = GameManagerInternal::toUpper(propertyCode);
    if (code.empty()) {
        code = GameManagerInternal::toUpper(display->getInput("Masukkan kode petak: "));
    }

    Property* property = board.getPropertyByCode(code);
    if (property == 0) {
        throw std::runtime_error("Petak \"" + code + "\" tidak ditemukan atau bukan properti.");
    }

    std::ostringstream oss;
    const std::string title = "[" + GameManagerInternal::propertyGroupLabel(*property) + "] "
        + GameManagerInternal::upperPrettyName(property->getName())
        + " (" + property->getCode() + ")";

    oss << "+==================================+\n";
    oss << "| " << centerText("AKTA KEPEMILIKAN", DEED_WIDTH) << " |\n";
    oss << "| " << centerText(title, DEED_WIDTH) << " |\n";
    oss << "+==================================+\n";
    appendLabelValue(oss, "Harga Beli", GameManagerInternal::formatMoney(property->getPurchasePrice()));
    appendLabelValue(oss, "Nilai Gadai", GameManagerInternal::formatMoney(property->getMortgageValue()));
    oss << "+----------------------------------+\n";

    Street* street = dynamic_cast<Street*>(property);
    if (street != 0) {
        const std::array<int, 6>& rents = street->getRentTable();
        appendLabelValue(oss, "Sewa (unimproved)", GameManagerInternal::formatMoney(rents[0] * (street->isMonopoly() ? 2 : 1)));
        appendLabelValue(oss, "Sewa (1 rumah)", GameManagerInternal::formatMoney(rents[1]));
        appendLabelValue(oss, "Sewa (2 rumah)", GameManagerInternal::formatMoney(rents[2]));
        appendLabelValue(oss, "Sewa (3 rumah)", GameManagerInternal::formatMoney(rents[3]));
        appendLabelValue(oss, "Sewa (4 rumah)", GameManagerInternal::formatMoney(rents[4]));
        appendLabelValue(oss, "Sewa (hotel)", GameManagerInternal::formatMoney(rents[5]));
        oss << "+----------------------------------+\n";
        appendLabelValue(oss, "Harga Rumah", GameManagerInternal::formatMoney(street->getHouseCost()));
        appendLabelValue(oss, "Harga Hotel", GameManagerInternal::formatMoney(street->getHotelCost()));
    } else {
        Railroad* railroad = dynamic_cast<Railroad*>(property);
        Utility* utility = dynamic_cast<Utility*>(property);
        if (railroad != 0) {
            const std::map<int, int>& rents = railroad->getRentByOwnedCount();
            appendLabelValue(oss, "Sewa (1 stasiun)", GameManagerInternal::formatMoney(rents.find(1) != rents.end() ? rents.find(1)->second : 0));
            appendLabelValue(oss, "Sewa (2 stasiun)", GameManagerInternal::formatMoney(rents.find(2) != rents.end() ? rents.find(2)->second : 0));
            appendLabelValue(oss, "Sewa (3 stasiun)", GameManagerInternal::formatMoney(rents.find(3) != rents.end() ? rents.find(3)->second : 0));
            appendLabelValue(oss, "Sewa (4 stasiun)", GameManagerInternal::formatMoney(rents.find(4) != rents.end() ? rents.find(4)->second : 0));
        } else if (utility != 0) {
            const std::map<int, int>& multipliers = utility->getMultiplierByOwnedCount();
            appendLabelValue(oss, "Sewa (1 utilitas)", "x" + std::to_string(multipliers.find(1) != multipliers.end() ? multipliers.find(1)->second : 0) + " total dadu");
            appendLabelValue(oss, "Sewa (2 utilitas)", "x" + std::to_string(multipliers.find(2) != multipliers.end() ? multipliers.find(2)->second : 0) + " total dadu");
        }
    }

    oss << "+==================================+\n";
    appendDeedLine(oss, "Status : " + GameManagerInternal::propertyStatusLabel(*property));
    const std::string festivalInfo = GameManagerInternal::festivalStatusLabel(*property);
    if (!festivalInfo.empty()) {
        appendDeedLine(oss, festivalInfo);
    }
    oss << "+==================================+";

    display->printMessage(oss.str());
}
