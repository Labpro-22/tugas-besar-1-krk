#include "core/game_manager/GameManager.hpp"

#include <cstdlib>
#include <sstream>
#include <stdexcept>

#include "GameManagerInternal.hpp"

void GameManager::handleCommand(const std::string& commandLine) {
    std::stringstream ss(commandLine);
    std::string command;
    ss >> command;
    command = GameManagerInternal::toUpper(command);

    if (command == "HELP") commandHelp();
    else if (command == "CETAK_PAPAN") commandPrintBoard();
    else if (command == "LEMPAR_DADU") commandRollDice();
    else if (command == "ATUR_DADU") {
        int a = 0;
        int b = 0;
        if (!(ss >> a >> b)) {
            throw std::runtime_error("Format ATUR_DADU adalah ATUR_DADU X Y.");
        }
        commandSetDice(a, b);
    }
    else if (command == "NEXT") commandNext();
    else if (command == "BAYAR_DENDA") commandPayJailFine();
    else if (command == "CETAK_AKTA") {
        std::string code;
        ss >> code;
        commandPrintDeed(code);
    }
    else if (command == "CETAK_PROPERTI") commandPrintProperties();
    else if (command == "GADAI") {
        std::string code;
        ss >> code;
        commandMortgage(code);
    }
    else if (command == "TEBUS") {
        std::string code;
        ss >> code;
        commandUnmortgage(code);
    }
    else if (command == "BANGUN") {
        std::string code;
        ss >> code;
        commandBuild(code);
    }
    else if (command == "SIMPAN") {
        std::string path;
        ss >> path;
        commandSave(path);
    }
    else if (command == "CETAK_LOG") {
        int count = -1;
        if (ss >> count) {
            commandPrintLog(count);
        } else {
            commandPrintLog();
        }
    }
    else if (command == "GUNAKAN_KEMAMPUAN") {
        std::size_t idx = 0;
        ss >> idx;
        if (idx == 0 && display != 0) {
            idx = static_cast<std::size_t>(std::atoi(display->getInput("Skill card index: ").c_str()));
        }
        commandUseSkillCard(idx == 0 ? 0 : idx - 1);
    }
    else throw std::runtime_error("Unknown command.");
}
