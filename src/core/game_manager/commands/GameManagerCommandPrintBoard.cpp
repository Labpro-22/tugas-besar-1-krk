#include "core/game_manager/GameManager.hpp"

#include <sstream>
#include <string>
#include <vector>

#include "models/base/Property.hpp"
#include "models/tiles/Street.hpp"
#include "models/base/Tile.hpp"
#include "models/tiles/Utility.hpp"

namespace {

const int CELL_WIDTH = 10;
const int CENTER_WIDTH = 98;

std::string padCell(const std::string& text, int width = CELL_WIDTH) {
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

std::string colorTagForTile(Tile* tile) {
    Utility* utility = dynamic_cast<Utility*>(tile);
    if (utility != 0) {
        return "AB";
    }

    Street* street = dynamic_cast<Street*>(tile);
    if (street == 0) {
        return "DF";
    }

    const std::string color = street->getColorGroup();
    if (color == "COKLAT") return "CK";
    if (color == "BIRU_MUDA") return "BM";
    if (color == "PINK") return "PK";
    if (color == "ORANGE") return "OR";
    if (color == "MERAH") return "MR";
    if (color == "KUNING") return "KN";
    if (color == "HIJAU") return "HJ";
    if (color == "BIRU_TUA") return "BT";
    return "DF";
}

int playerDisplayIndex(const std::vector<Player*>& players, const Player* player) {
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i] == player) {
            return static_cast<int>(i) + 1;
        }
    }
    return 0;
}

std::string tileTopLine(Tile* tile) {
    return padCell("[" + colorTagForTile(tile) + "] " + tile->getCode());
}

std::string buildLevelMarker(int level) {
    if (level >= 5) {
        return "*";
    }
    if (level > 0) {
        return std::string(static_cast<std::size_t>(level), '^');
    }
    return "";
}

std::string joinParts(const std::vector<std::string>& parts, const std::string& separator) {
    std::string result;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (parts[i].empty()) {
            continue;
        }
        if (!result.empty()) {
            result += separator;
        }
        result += parts[i];
    }
    return result;
}

std::string tileBottomLine(Tile* tile, const std::vector<Player*>& players) {
    if (tile->getPosition() == 11) {
        int inCount = 0;
        int visitCount = 0;
        for (std::size_t i = 0; i < players.size(); ++i) {
            if (players[i]->getPosition() == 11) {
                if (players[i]->isInJail()) {
                    ++inCount;
                } else {
                    ++visitCount;
                }
            }
        }

        std::ostringstream oss;
        oss << "IN:" << inCount << " V:" << visitCount;
        return padCell(oss.str());
    }

    std::string line;
    std::string ownerPart;
    std::string buildingPart;
    std::string tokenPart;
    Property* property = dynamic_cast<Property*>(tile);
    if (property != 0 && property->getOwner() != 0) {
        int ownerIndex = playerDisplayIndex(players, property->getOwner());
        if (ownerIndex > 0) {
            ownerPart = "P" + std::to_string(ownerIndex);
        }

        Street* street = dynamic_cast<Street*>(property);
        if (street != 0) {
            buildingPart = buildLevelMarker(street->getBuildingLevel());
        }
    }

    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i]->getPosition() == tile->getPosition() && tile->getPosition() != 11) {
            tokenPart += "(" + std::to_string(static_cast<int>(i) + 1) + ")";
        }
    }

    const std::vector<std::string> candidates = {
        joinParts({ownerPart, buildingPart, tokenPart}, " "),
        joinParts({ownerPart, buildingPart}, " ") + (!tokenPart.empty() ? " " + tokenPart : ""),
        joinParts({ownerPart, tokenPart}, " "),
        ownerPart + (!buildingPart.empty() ? buildingPart : "") + tokenPart,
        ownerPart + tokenPart,
        joinParts({ownerPart, buildingPart}, " "),
        ownerPart + (!buildingPart.empty() ? buildingPart : ""),
        tokenPart
    };

    for (std::size_t i = 0; i < candidates.size(); ++i) {
        if (!candidates[i].empty() && static_cast<int>(candidates[i].size()) <= CELL_WIDTH) {
            line = candidates[i];
            break;
        }
    }

    if (line.empty()) {
        line = ownerPart.empty() ? tokenPart : ownerPart;
    }

    if (!ownerPart.empty() && tokenPart.empty() && static_cast<int>(line.size()) < CELL_WIDTH) {
        return padCell(line);
    }
    if (ownerPart.empty() && !line.empty() && static_cast<int>(line.size()) < CELL_WIDTH) {
        return centerText(line, CELL_WIDTH);
    }
    return padCell(line);
}

std::string horizontalBorder(int cellCount) {
    std::string line;
    for (int i = 0; i < cellCount; ++i) {
        line += "+----------";
    }
    line += "+";
    return line;
}

std::string renderSideLine(Tile* left, const std::string& center, Tile* right, bool topLine, const std::vector<Player*>& players) {
    return "|" + (topLine ? tileTopLine(left) : tileBottomLine(left, players)) + "|"
         + centerText(center, CENTER_WIDTH) + "|"
         + (topLine ? tileTopLine(right) : tileBottomLine(right, players)) + "|";
}

std::string renderSeparatorLine(const std::string& center) {
    return "+----------+" + centerText(center, CENTER_WIDTH) + "+----------+";
}

}  // namespace

void GameManager::commandPrintBoard() const {
    if (display == 0) return;

    const int topRow[11] = {21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
    const int bottomRow[11] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    const int leftColumn[9] = {20, 19, 18, 17, 16, 15, 14, 13, 12};
    const int rightColumn[9] = {32, 33, 34, 35, 36, 37, 38, 39, 40};

    struct CenterRow {
        std::string top;
        std::string bottom;
        std::string separator;
    };

    const CenterRow centerRows[9] = {
        {"", "", ""},
        {"==================================", "||          NIMONSPOLI          ||", "=================================="},
        {"", "TURN " + std::to_string(currentTurn) + " / " + std::to_string(getMaxTurn()), ""},
        {"", "----------------------------------", "LEGENDA KEPEMILIKAN & STATUS"},
        {"P1-P4 : Properti milik Pemain 1-4", "^     : Rumah Level 1", "^^    : Rumah Level 2"},
        {"^^^   : Rumah Level 3", "* : Hotel (Maksimal)", "(1)-(4): Bidak (IN=Tahanan, V=Mampir)"},
        {"----------------------------------", "KODE WARNA:", "[CK]=Coklat    [MR]=Merah"},
        {"[BM]=Biru Muda [KN]=Kuning", "[PK]=Pink      [HJ]=Hijau", "[OR]=Orange    [BT]=Biru Tua"},
        {"[DF]=Aksi      [AB]=Utilitas", "", ""}
    };

    std::ostringstream oss;
    oss << horizontalBorder(11) << '\n';
    for (int i = 0; i < 11; ++i) {
        oss << "|" << tileTopLine(board.getTile(topRow[i]));
    }
    oss << "|\n";
    for (int i = 0; i < 11; ++i) {
        oss << "|" << tileBottomLine(board.getTile(topRow[i]), players);
    }
    oss << "|\n";
    oss << horizontalBorder(11) << '\n';

    for (int row = 0; row < 9; ++row) {
        Tile* left = board.getTile(leftColumn[row]);
        Tile* right = board.getTile(rightColumn[row]);
        oss << renderSideLine(left, centerRows[row].top, right, true, players) << '\n';
        oss << renderSideLine(left, centerRows[row].bottom, right, false, players) << '\n';
        if (row < 8) {
            oss << renderSeparatorLine(centerRows[row].separator) << '\n';
        }
    }

    oss << horizontalBorder(11) << '\n';

    for (int i = 0; i < 11; ++i) {
        oss << "|" << tileTopLine(board.getTile(bottomRow[i]));
    }
    oss << "|\n";
    for (int i = 0; i < 11; ++i) {
        oss << "|" << tileBottomLine(board.getTile(bottomRow[i]), players);
    }
    oss << "|\n";
    oss << horizontalBorder(11);
    display->printMessage(oss.str());
}
