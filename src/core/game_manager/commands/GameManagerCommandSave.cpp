#include "core/game_manager/GameManager.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>

namespace {

std::filesystem::path resolveSaveOutputPath(const std::string& input) {
    if (input.empty()) {
        return std::filesystem::path("save") / "savegame.txt";
    }

    const std::filesystem::path path(input);
    if (path.is_absolute() || path.has_parent_path()) {
        return path;
    }
    return std::filesystem::path("save") / path;
}

}

void GameManager::commandSave(const std::string& saveFilePath) const {
    if (hasRolledThisTurn || hasUsedSkillThisTurn) {
        throw std::runtime_error("Save is only allowed before rolling and before using a skill card this turn.");
    }

    std::string requestedPath = saveFilePath;
    if (requestedPath.empty() && display != 0) {
        requestedPath = display->getInput("Save file name/path [save/savegame.txt]: ");
    }

    const std::filesystem::path resolvedPath = resolveSaveOutputPath(requestedPath);
    if (resolvedPath.has_parent_path()) {
        std::filesystem::create_directories(resolvedPath.parent_path());
    }

    if (display != 0) {
        display->printMessage("Menyimpan permainan...");
    }
    saveGame(resolvedPath.string());
    if (display != 0) {
        display->printMessage("Permainan berhasil disimpan ke: " + resolvedPath.generic_string());
    }
}
