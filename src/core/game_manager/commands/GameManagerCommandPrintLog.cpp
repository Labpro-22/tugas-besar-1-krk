#include "core/game_manager/GameManager.hpp"

#include <sstream>
#include <vector>

void GameManager::commandPrintLog(int lastEntries) const {
    if (display == 0) return;

    std::vector<std::string> lines = logger.toDisplayLines();
    std::ostringstream oss;
    std::size_t start = 0;
    if (lastEntries >= 0 && static_cast<std::size_t>(lastEntries) < lines.size()) {
        start = lines.size() - static_cast<std::size_t>(lastEntries);
        oss << "=== Log Transaksi (" << lastEntries << " Terakhir) ===\n\n";
    } else {
        oss << "=== Log Transaksi Penuh ===\n\n";
    }

    for (std::size_t i = start; i < lines.size(); ++i) {
        oss << lines[i] << '\n';
    }
    display->printMessage(oss.str());
}
