#include "core/game_manager/GameManager.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <sstream>

#include "GameManagerInternal.hpp"
#include "models/tiles/Action.hpp"
#include "models/base/Property.hpp"
#include "models/tiles/Railroad.hpp"
#include "models/tiles/Special.hpp"
#include "models/tiles/Street.hpp"
#include "models/tiles/Utility.hpp"

namespace {

std::filesystem::path resolveSaveInputPath(const std::string& input) {
    if (input.empty()) {
        return std::filesystem::path("save");
    }

    const std::filesystem::path path(input);
    if (path.is_absolute() || path.has_parent_path()) {
        return path;
    }
    return std::filesystem::path("save") / path;
}

std::vector<std::filesystem::path> listSaveFiles(const std::filesystem::path& directory) {
    std::vector<std::filesystem::path> files;
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        return files;
    }

    for (std::filesystem::directory_iterator it(directory); it != std::filesystem::directory_iterator(); ++it) {
        if (it->is_regular_file()) {
            files.push_back(it->path());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

std::filesystem::path chooseSaveFile(DisplayHandler* display, const std::filesystem::path& directory) {
    const std::vector<std::filesystem::path> files = listSaveFiles(directory);
    if (files.empty()) {
        throw std::runtime_error("Tidak ada file save di folder: " + directory.generic_string());
    }

    std::ostringstream oss;
    oss << "Daftar file save di " << directory.generic_string() << ":\n";
    for (std::size_t i = 0; i < files.size(); ++i) {
        oss << i + 1 << ". " << files[i].filename().generic_string() << '\n';
    }
    display->printMessage(oss.str());

    const std::string input = display->getInput("Pilih nomor file save atau ketik nama file: ");
    if (input.empty()) {
        return files[0];
    }

    bool numeric = true;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(input[i]))) {
            numeric = false;
            break;
        }
    }

    if (numeric) {
        int choice = std::atoi(input.c_str());
        if (choice >= 1 && static_cast<std::size_t>(choice) <= files.size()) {
            return files[static_cast<std::size_t>(choice - 1)];
        }
    }

    const std::filesystem::path chosen(input);
    if (chosen.is_absolute() || chosen.has_parent_path()) {
        return chosen;
    }
    return directory / chosen;
}

ActionTileDefinition buildFallbackActionTile(int id,
                                             const std::string& code,
                                             const std::string& name,
                                             bool isSpecial,
                                             ActionType actionType,
                                             SpecialType specialType) {
    ActionTileDefinition def;
    def.id = id;
    def.code = code;
    def.name = name;
    def.colorGroup = "DEFAULT";
    def.isSpecial = isSpecial;
    def.actionType = actionType;
    def.specialType = specialType;
    return def;
}

Tile* createActionTile(const ActionTileDefinition& definition) {
    if (definition.isSpecial) {
        return new Special(definition.code, definition.name, definition.id, definition.specialType);
    }
    return new Action(definition.code, definition.name, definition.id, definition.actionType);
}

const ActionTileDefinition* findActionTileDefinition(const GameConfig& config, int position) {
    for (std::size_t i = 0; i < config.actionTiles.size(); ++i) {
        if (config.actionTiles[i].id == position) {
            return &config.actionTiles[i];
        }
    }
    return 0;
}

}

void GameManager::loadConfig(const std::string& configDirectory) {
    gameConfig = configManager.loadGameConfig(configDirectory);
}

void GameManager::buildDefaultBoard() {
    board.clear();

    if (gameConfig.properties.empty()) {
        auto addStreet = [&](int id, const char* code, const char* name, const char* color,
                             int price, int mortgage, int house, int hotel,
                             int r0, int r1, int r2, int r3, int r4, int r5, int groupSize) {
            PropertyDefinition def;
            def.id = id;
            def.code = code;
            def.name = name;
            def.type = PropertyType::STREET;
            def.colorGroup = color;
            def.purchasePrice = price;
            def.mortgageValue = mortgage;
            def.houseCost = house;
            def.hotelCost = hotel;
            def.colorGroupSize = groupSize;
            def.rentTable.push_back(r0);
            def.rentTable.push_back(r1);
            def.rentTable.push_back(r2);
            def.rentTable.push_back(r3);
            def.rentTable.push_back(r4);
            def.rentTable.push_back(r5);
            gameConfig.properties.push_back(def);
        };
        auto addRailroad = [&](int id, const char* code, const char* name, int mortgage) {
            PropertyDefinition def;
            def.id = id;
            def.code = code;
            def.name = name;
            def.type = PropertyType::RAILROAD;
            def.colorGroup = "DEFAULT";
            def.purchasePrice = 0;
            def.mortgageValue = mortgage;
            gameConfig.properties.push_back(def);
        };
        auto addUtility = [&](int id, const char* code, const char* name, int mortgage) {
            PropertyDefinition def;
            def.id = id;
            def.code = code;
            def.name = name;
            def.type = PropertyType::UTILITY;
            def.colorGroup = "ABU_ABU";
            def.purchasePrice = 0;
            def.mortgageValue = mortgage;
            gameConfig.properties.push_back(def);
        };

        addStreet(2,"GRT","Garut","COKLAT",60,40,20,50,2,10,30,90,160,250,2);
        addStreet(4,"TSK","Tasikmalaya","COKLAT",60,40,50,50,4,20,60,180,320,450,2);
        addRailroad(6,"GBR","Stasiun_Gambir",20);
        addStreet(7,"BGR","Bogor","BIRU_MUDA",100,80,20,50,6,30,90,270,400,550,3);
        addStreet(9,"DPK","Depok","BIRU_MUDA",100,80,20,50,6,30,90,270,400,550,3);
        addStreet(10,"BKS","Bekasi","BIRU_MUDA",120,90,20,50,8,40,100,300,450,600,3);
        addStreet(12,"MGL","Magelang","PINK",140,100,100,100,10,50,150,450,625,750,3);
        addUtility(13,"PLN","PLN",25);
        addStreet(14,"SOL","Solo","PINK",140,100,100,100,10,50,150,450,625,750,3);
        addStreet(15,"YOG","Yogyakarta","PINK",160,120,100,100,12,60,180,500,700,900,3);
        addRailroad(16,"STB","Stasiun_Bandung",40);
        addStreet(17,"MAL","Malang","ORANGE",180,135,100,100,14,70,200,550,750,950,3);
        addStreet(19,"SMG","Semarang","ORANGE",180,140,100,100,14,70,200,550,750,950,3);
        addStreet(20,"SBY","Surabaya","ORANGE",200,150,100,100,16,80,220,600,800,1000,3);
        addStreet(22,"MKS","Makassar","MERAH",220,175,150,150,18,90,250,700,875,1050,3);
        addStreet(24,"BLP","Balikpapan","MERAH",220,175,150,150,18,90,250,700,875,1050,3);
        addStreet(25,"MND","Manado","MERAH",240,190,150,150,20,100,300,750,925,1100,3);
        addRailroad(26,"TUG","Stasiun_Tugu",50);
        addStreet(27,"PLB","Palembang","KUNING",260,200,150,150,22,110,330,800,975,1150,3);
        addStreet(28,"PKB","Pekanbaru","KUNING",260,210,150,150,22,110,330,800,975,1150,3);
        addUtility(29,"PAM","PAM",60);
        addStreet(30,"MED","Medan","KUNING",280,225,150,150,24,120,360,850,1025,1200,3);
        addStreet(32,"BDG","Bandung","HIJAU",300,250,200,200,26,130,390,900,1100,1275,3);
        addStreet(33,"DEN","Denpasar","HIJAU",300,260,200,200,26,130,390,900,1100,1275,3);
        addStreet(35,"MTR","Mataram","HIJAU",320,280,200,200,28,150,450,1000,1200,1400,3);
        addRailroad(36,"GUB","Stasiun_Gubeng",120);
        addStreet(38,"JKT","Jakarta","BIRU_TUA",350,300,200,200,35,175,500,1100,1300,1500,2);
        addStreet(40,"IKN","Ibu_Kota_Nusantara","BIRU_TUA",400,350,200,200,50,200,600,1400,1700,2000,2);
        gameConfig.railroadRentByCount[1] = 25;
        gameConfig.railroadRentByCount[2] = 50;
        gameConfig.railroadRentByCount[3] = 100;
        gameConfig.railroadRentByCount[4] = 200;
        gameConfig.utilityMultiplierByCount[1] = 4;
        gameConfig.utilityMultiplierByCount[2] = 10;
    }

    if (gameConfig.actionTiles.empty()) {
        gameConfig.actionTiles.push_back(buildFallbackActionTile(1, "GO", "Petak_Mulai", true, ActionType::COMMUNITY_CHEST, SpecialType::GO));
        gameConfig.actionTiles.push_back(buildFallbackActionTile(3, "DNU", "Dana_Umum", false, ActionType::COMMUNITY_CHEST, SpecialType::FREE_PARKING));
        gameConfig.actionTiles.push_back(buildFallbackActionTile(5, "PPH", "Pajak_Penghasilan", false, ActionType::TAX_PPH, SpecialType::FREE_PARKING));
        gameConfig.actionTiles.push_back(buildFallbackActionTile(8, "FES", "Festival", false, ActionType::FESTIVAL, SpecialType::FREE_PARKING));
        gameConfig.actionTiles.push_back(buildFallbackActionTile(11, "PEN", "Penjara", true, ActionType::COMMUNITY_CHEST, SpecialType::JAIL));
        gameConfig.actionTiles.push_back(buildFallbackActionTile(18, "DNU", "Dana_Umum", false, ActionType::COMMUNITY_CHEST, SpecialType::FREE_PARKING));
        gameConfig.actionTiles.push_back(buildFallbackActionTile(21, "BBP", "Bebas_Parkir", true, ActionType::COMMUNITY_CHEST, SpecialType::FREE_PARKING));
        gameConfig.actionTiles.push_back(buildFallbackActionTile(23, "KSP", "Kesempatan", false, ActionType::CHANCE, SpecialType::FREE_PARKING));
        gameConfig.actionTiles.push_back(buildFallbackActionTile(31, "PPJ", "Petak_Pergi_ke_Penjara", true, ActionType::COMMUNITY_CHEST, SpecialType::GO_TO_JAIL));
        gameConfig.actionTiles.push_back(buildFallbackActionTile(34, "FES", "Festival", false, ActionType::FESTIVAL, SpecialType::FREE_PARKING));
        gameConfig.actionTiles.push_back(buildFallbackActionTile(37, "KSP", "Kesempatan", false, ActionType::CHANCE, SpecialType::FREE_PARKING));
        gameConfig.actionTiles.push_back(buildFallbackActionTile(39, "PBM", "Pajak_Barang_Mewah", false, ActionType::TAX_PBM, SpecialType::FREE_PARKING));
    }

    for (int position = 1; position <= 40; ++position) {
        PropertyDefinition* found = 0;
        for (std::size_t i = 0; i < gameConfig.properties.size(); ++i) {
            if (gameConfig.properties[i].id == position) {
                found = &gameConfig.properties[i];
                break;
            }
        }
        if (found != 0) {
            if (found->type == PropertyType::STREET) {
                std::array<int, 6> rents = {{0,0,0,0,0,0}};
                for (std::size_t i = 0; i < found->rentTable.size() && i < 6; ++i) {
                    rents[i] = found->rentTable[i];
                }
                board.addTile(new Street(found->code, found->name, found->id, found->colorGroup, found->colorGroupSize,
                                         found->purchasePrice, found->mortgageValue, found->houseCost, found->hotelCost, rents));
            } else if (found->type == PropertyType::RAILROAD) {
                board.addTile(new Railroad(found->code, found->name, found->id, found->mortgageValue, gameConfig.railroadRentByCount));
            } else {
                board.addTile(new Utility(found->code, found->name, found->id, found->mortgageValue, gameConfig.utilityMultiplierByCount));
            }
        } else {
            const ActionTileDefinition* actionTile = findActionTileDefinition(gameConfig, position);
            if (actionTile != 0) {
                board.addTile(createActionTile(*actionTile));
            } else {
                board.addTile(new Special("NA", "Unknown", position, SpecialType::FREE_PARKING));
            }
        }
    }
}

void GameManager::buildDefaultChanceDeck() {
    chanceDeck.clear();
    chanceDeck.addCard(new ChanceCard("Stasiun Terdekat", "Pergi ke stasiun terdekat.", CardEffectType::MOVE_TO_NEAREST_RAILROAD));
    chanceDeck.addCard(new ChanceCard("Mundur 3 Petak", "Mundur 3 petak.", CardEffectType::MOVE_RELATIVE, -3));
    chanceDeck.addCard(new ChanceCard("Masuk Penjara", "Masuk penjara.", CardEffectType::GO_TO_JAIL));
    chanceDeck.shuffle();
}

void GameManager::buildDefaultCommunityDeck() {
    communityDeck.clear();
    communityDeck.addCard(new CommunityChestCard("Hari Ulang Tahun", "Dapatkan M100 dari setiap pemain.", CardEffectType::RECEIVE_FROM_EACH_PLAYER, 100));
    communityDeck.addCard(new CommunityChestCard("Biaya Dokter", "Bayar M700.", CardEffectType::PAY_MONEY, 700));
    communityDeck.addCard(new CommunityChestCard("Nyaleg", "Bayar M200 kepada setiap pemain.", CardEffectType::PAY_EACH_PLAYER, 200));
    communityDeck.shuffle();
}

void GameManager::buildDefaultSkillDeck() {
    skillDeck.clear();
    clearDiscardedSkillCards();
    for (int i = 0; i < 4; ++i) skillDeck.addCard(new SkillCard("MoveCard", "Move forward.", "MoveCard", (std::rand() % 12) + 1, 0, true));
    for (int i = 0; i < 3; ++i) skillDeck.addCard(new SkillCard("DiscountCard", "Discount for one turn.", "DiscountCard", 10 + (std::rand() % 41), 1, true));
    for (int i = 0; i < 2; ++i) skillDeck.addCard(new SkillCard("ShieldCard", "Shield for one turn.", "ShieldCard", 0, 1, true));
    for (int i = 0; i < 2; ++i) skillDeck.addCard(new SkillCard("TeleportCard", "Teleport to any tile.", "TeleportCard", 0, 0, true));
    for (int i = 0; i < 2; ++i) skillDeck.addCard(new SkillCard("LassoCard", "Pull nearest opponent ahead.", "LassoCard", 0, 0, true));
    for (int i = 0; i < 2; ++i) skillDeck.addCard(new SkillCard("DemolitionCard", "Destroy buildings on one enemy property.", "DemolitionCard", 0, 0, true));
    skillDeck.shuffle();
}

void GameManager::createPlayers(const std::vector<std::string>& playerNames) {
    clearPlayers();
    for (std::size_t i = 0; i < playerNames.size(); ++i) {
        players.push_back(new Player(playerNames[i], gameConfig.misc.startingBalance));
    }
    for (std::size_t i = players.size(); i > 1; --i) {
        std::size_t j = static_cast<std::size_t>(std::rand()) % i;
        std::swap(players[i - 1], players[j]);
    }
    currentPlayerIndex = 0;
    currentTurn = 1;
}

void GameManager::startNewGame(const std::vector<std::string>& playerNames, const std::string& configDirectory) {
    loadConfig(configDirectory);
    buildDefaultBoard();
    buildDefaultChanceDeck();
    buildDefaultCommunityDeck();
    buildDefaultSkillDeck();
    logger.clear();
    createPlayers(playerNames);
    useManualDice = false;
    manualDiceValue = std::make_pair(1, 1);
    hasRolledThisTurn = false;
    hasUsedSkillThisTurn = false;
    consecutiveDoubles = 0;
    turnPhase = TurnPhase::START_TURN;
    turnShouldEnd = false;
    currentRollWasDouble = false;
}

void GameManager::loadSavedGame(const std::string& saveFilePath, const std::string& configDirectory) {
    loadConfig(configDirectory);
    buildDefaultBoard();
    buildDefaultChanceDeck();
    buildDefaultCommunityDeck();
    buildDefaultSkillDeck();
    GameSaveData save = configManager.loadGameState(saveFilePath);
    applySaveData(save);
    useManualDice = false;
    manualDiceValue = std::make_pair(1, 1);
    hasRolledThisTurn = false;
    hasUsedSkillThisTurn = false;
    consecutiveDoubles = 0;
    turnPhase = TurnPhase::START_TURN;
    turnShouldEnd = false;
    currentRollWasDouble = false;
}

void GameManager::saveGame(const std::string& saveFilePath) const {
    configManager.saveGameState(saveFilePath, buildSaveData());
}

void GameManager::start() {
    if (display == 0) {
        throw std::runtime_error("Display handler not set.");
    }
    std::string configDir = display->getInput("Config directory [config]: ");
    if (configDir.empty()) configDir = "config";
    std::string choice = GameManagerInternal::toUpper(display->getInput("Type NEW or LOAD: "));
    if (choice == "LOAD") {
        std::filesystem::create_directories("save");
        std::string input = display->getInput("Save file path [save/]: ");
        std::filesystem::path path = resolveSaveInputPath(input);
        if (input.empty() || (std::filesystem::exists(path) && std::filesystem::is_directory(path))) {
            path = chooseSaveFile(display, path);
        }
        loadSavedGame(path.string(), configDir);
    } else {
        int count = std::atoi(display->getInput("Number of players (2-4): ").c_str());
        if (count < 2) count = 2;
        if (count > 4) count = 4;
        std::vector<std::string> names;
        for (int i = 0; i < count; ++i) {
            names.push_back(display->getInput("Player " + std::to_string(i + 1) + " name: "));
            if (names.back().empty()) names.back() = "P" + std::to_string(i + 1);
        }
        startNewGame(names, configDir);
    }
    gameLoop();
}
