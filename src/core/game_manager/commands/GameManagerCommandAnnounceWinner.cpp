#include "core/game_manager/GameManager.hpp"

#include <algorithm>
#include <sstream>
#include <vector>

void GameManager::commandAnnounceWinner() const {
    if (display == 0 || players.empty()) return;

    std::vector<const Player*> finalists;
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (!players[i]->isBankrupt()) {
            finalists.push_back(players[i]);
        }
    }
    if (finalists.empty()) {
        return;
    }
    if (finalists.size() == 1) {
        display->printMessage("Pemenang: " + finalists[0]->getName());
        return;
    }

    auto filterByBest = [&](int (*metric)(const Player&)) {
        int best = metric(*finalists[0]);
        for (std::size_t i = 1; i < finalists.size(); ++i) {
            best = std::max(best, metric(*finalists[i]));
        }

        std::vector<const Player*> filtered;
        for (std::size_t i = 0; i < finalists.size(); ++i) {
            if (metric(*finalists[i]) == best) {
                filtered.push_back(finalists[i]);
            }
        }
        finalists = filtered;
    };

    filterByBest([](const Player& player) { return player.getMoney(); });
    if (finalists.size() > 1) {
        filterByBest([](const Player& player) { return player.countOwnedProperties(); });
    }
    if (finalists.size() > 1) {
        filterByBest([](const Player& player) { return player.countOwnedSkillCards(); });
    }

    std::ostringstream oss;
    oss << "Pemenang: ";
    for (std::size_t i = 0; i < finalists.size(); ++i) {
        if (i > 0) {
            oss << ", ";
        }
        oss << finalists[i]->getName();
    }
    display->printMessage(oss.str());
}
