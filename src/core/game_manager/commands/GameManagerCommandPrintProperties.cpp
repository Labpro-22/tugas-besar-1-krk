#include "core/game_manager/GameManager.hpp"

#include <map>
#include <sstream>
#include <vector>

#include "../GameManagerInternal.hpp"
#include "models/base/Property.hpp"
#include "models/tiles/Street.hpp"

void GameManager::commandPrintProperties() const {
    if (display == 0) return;

    const Player& player = *players[static_cast<std::size_t>(currentPlayerIndex)];
    const std::vector<Property*>& props = player.getOwnedProperties();
    const std::vector<SkillCard*>& cards = player.getInventory().getSkillCards();

    if (props.empty()) {
        display->printMessage("Kamu belum memiliki properti apapun.");
        return;
    }

    std::map<std::string, std::vector<Property*> > grouped;
    std::vector<std::string> groupOrder;
    int totalPropertyWealth = 0;
    for (std::size_t i = 0; i < props.size(); ++i) {
        std::string group = GameManagerInternal::propertyGroupLabel(*props[i]);
        if (grouped.find(group) == grouped.end()) {
            groupOrder.push_back(group);
        }
        grouped[group].push_back(props[i]);
        totalPropertyWealth += props[i]->getAssetValue();
    }

    std::ostringstream oss;
    oss << "=== Properti Milik: " << player.getName() << " ===\n\n";

    for (std::size_t groupIndex = 0; groupIndex < groupOrder.size(); ++groupIndex) {
        const std::string& group = groupOrder[groupIndex];
        oss << "[" << group << "]\n";

        const std::vector<Property*>& list = grouped[group];
        for (std::size_t i = 0; i < list.size(); ++i) {
            Property* property = list[i];
            Street* street = dynamic_cast<Street*>(property);
            int shownValue = property->getPurchasePrice() > 0 ? property->getPurchasePrice() : property->getAssetValue();

            oss << "  - "
                << GameManagerInternal::prettyName(property->getName())
                << " (" << property->getCode() << ")";

            if (street != 0 && street->getBuildingLevel() > 0) {
                oss << "  " << GameManagerInternal::buildingLevelLabel(street->getBuildingLevel());
            }

            oss << "  " << GameManagerInternal::formatMoney(shownValue)
                << "  " << GameManagerInternal::propertyStatusToString(property->getStatus());

            if (property->isMortgaged()) {
                oss << " [M]";
            }

            const std::string festivalInfo = GameManagerInternal::festivalStatusLabel(*property);
            if (!festivalInfo.empty()) {
                oss << "  (" << festivalInfo << ")";
            }
            oss << '\n';
        }
        oss << '\n';
    }

    oss << "Total kekayaan properti: " << GameManagerInternal::formatMoney(totalPropertyWealth);

    if (!cards.empty()) {
        oss << "\n\nKartu Kemampuan:\n";
        for (std::size_t i = 0; i < cards.size(); ++i) {
            oss << i + 1 << ". " << cards[i]->getName();
            if (cards[i]->getName() == "MoveCard") {
                oss << " - Maju " << cards[i]->getValue() << " petak";
            } else if (cards[i]->getName() == "DiscountCard") {
                oss << " - Diskon " << cards[i]->getValue() << "%";
            } else if (cards[i]->getName() == "ShieldCard") {
                oss << " - Kebal tagihan/sanksi";
            } else if (cards[i]->getName() == "TeleportCard") {
                oss << " - Pindah ke petak manapun";
            } else if (cards[i]->getName() == "LassoCard") {
                oss << " - Menarik lawan";
            } else if (cards[i]->getName() == "DemolitionCard") {
                oss << " - Menghancurkan bangunan lawan";
            }
            oss << '\n';
        }
    }

    display->printMessage(oss.str());
}
