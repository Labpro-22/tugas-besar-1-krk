#include "core/game_manager/GameManager.hpp"

#include <sstream>
#include <stdexcept>
#include <vector>

#include "GameManagerInternal.hpp"
#include "models/base/Property.hpp"
#include "models/tiles/Street.hpp"
#include "models/base/Tile.hpp"

GameManager* GameManager::instance = 0;

GameManager::GameManager()
    : display(0), currentPlayerIndex(0), currentTurn(1), consecutiveDoubles(0),
      useManualDice(false), manualDiceValue(std::make_pair(1, 1)), hasRolledThisTurn(false),
      hasUsedSkillThisTurn(false), turnPhase(TurnPhase::START_TURN), turnShouldEnd(false),
      currentRollWasDouble(false) {}

GameManager::~GameManager() {
    clearPlayers();
    clearDiscardedSkillCards();
}

GameManager* GameManager::getInstance() {
    if (instance == 0) {
        instance = new GameManager();
    }
    return instance;
}

void GameManager::setDisplayHandler(DisplayHandler* displayHandler) {
    display = displayHandler;
}

void GameManager::clearPlayers() {
    for (std::size_t i = 0; i < players.size(); ++i) {
        delete players[i];
    }
    players.clear();
}

void GameManager::clearDiscardedSkillCards() {
    for (std::size_t i = 0; i < discardedSkillCards.size(); ++i) {
        delete discardedSkillCards[i];
    }
    discardedSkillCards.clear();
}

Property* GameManager::findPropertyOrThrow(const std::string& propertyCode) const {
    Property* property = board.getPropertyByCode(propertyCode);
    if (property == 0) {
        throw std::runtime_error("Property not found: " + propertyCode);
    }
    return property;
}

Player* GameManager::findPlayerByName(const std::string& playerName) const {
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i]->getName() == playerName) {
            return players[i];
        }
    }
    return 0;
}

void GameManager::refreshFestivalStates(Player& player) {
    const std::vector<Property*>& owned = player.getOwnedProperties();
    for (std::size_t i = 0; i < owned.size(); ++i) {
        if (owned[i] != 0) {
            owned[i]->tickFestival();
        }
    }
}

void GameManager::handleAutomaticSkillDrop(Player& player) {
    if (!player.getInventory().isSkillCardFull()) {
        return;
    }
    std::size_t indexToDrop = 0;
    if (display != 0) {
        commandPrintProperties();
        std::ostringstream oss;
        oss << "Skill card limit reached. Choose card index to drop (1-"
            << player.getInventory().getSkillCardCount() << "): ";
        std::string input = display->getInput(oss.str());
        if (!input.empty()) {
            int chosen = std::atoi(input.c_str());
            if (chosen >= 1 && static_cast<std::size_t>(chosen) <= player.getInventory().getSkillCardCount()) {
                indexToDrop = static_cast<std::size_t>(chosen - 1);
            }
        }
    }
    SkillCard* removed = player.getInventory().takeSkillCard(indexToDrop);
    discardSkillCard(removed);
}

GameSaveData GameManager::buildSaveData() const {
    GameSaveData save;
    save.currentTurn = currentTurn;
    save.maxTurn = getMaxTurn();
    save.currentPlayerIndex = currentPlayerIndex;

    for (std::size_t i = 0; i < players.size(); ++i) {
        SavedPlayerState state;
        state.name = players[i]->getName();
        state.money = players[i]->getMoney();
        state.position = players[i]->getPosition();
        Tile* tile = board.getTile(players[i]->getPosition());
        state.tileCode = (tile != 0 ? tile->getCode() : "GO");
        state.status = players[i]->getStatus();
        state.jailTurnsSpent = players[i]->getJailTurnsSpent();
        const std::vector<SkillCard*>& cards = players[i]->getInventory().getSkillCards();
        for (std::size_t j = 0; j < cards.size(); ++j) {
            SavedSkillCardState c;
            c.cardName = cards[j]->getName();
            c.value = cards[j]->getValue();
            c.remainingDuration = cards[j]->getRemainingDuration();
            state.skillCards.push_back(c);
        }
        save.players.push_back(state);
        save.turnOrderNames.push_back(players[i]->getName());
    }

    std::vector<Property*> props = board.getAllProperties();
    for (std::size_t i = 0; i < props.size(); ++i) {
        SavedPropertyState st;
        st.code = props[i]->getCode();
        st.ownerName = props[i]->getOwner() != 0 ? props[i]->getOwner()->getName() : "BANK";
        st.propertyType = props[i]->getPropertyType();
        st.status = props[i]->getStatus();
        st.festivalMultiplier = props[i]->getFestivalMultiplier();
        st.festivalTurnsRemaining = props[i]->getFestivalTurnsRemaining();
        Street* street = dynamic_cast<Street*>(props[i]);
        st.buildingLevel = street != 0 ? street->getBuildingLevel() : 0;
        save.properties.push_back(st);
    }

    const std::vector<SkillCard*>& deckCards = skillDeck.getCards();
    for (std::size_t i = 0; i < deckCards.size(); ++i) {
        save.skillDeckState.skillDeckCardNames.push_back(deckCards[i]->getName());
    }
    save.logEntries = logger.getEntries();
    return save;
}

void GameManager::applySaveData(const GameSaveData& saveData) {
    currentTurn = saveData.currentTurn;
    currentPlayerIndex = saveData.currentPlayerIndex;
    logger.setEntries(saveData.logEntries);

    clearPlayers();
    for (std::size_t i = 0; i < saveData.players.size(); ++i) {
        Player* player = new Player(saveData.players[i].name, saveData.players[i].money);
        int restoredPosition = saveData.players[i].position;
        if (restoredPosition < 1 || restoredPosition > board.size()) {
            Tile* tile = 0;
            for (int pos = 1; pos <= board.size(); ++pos) {
                Tile* current = board.getTile(pos);
                if (current != 0 && current->getCode() == saveData.players[i].tileCode) {
                    tile = current;
                    break;
                }
            }
            restoredPosition = tile != 0 ? tile->getPosition() : 1;
        }
        player->setPosition(restoredPosition);
        player->setStatus(saveData.players[i].status);
        if (saveData.players[i].status == PlayerStatus::JAILED) {
            player->resetJailTurns();
            for (int t = 0; t < saveData.players[i].jailTurnsSpent; ++t) player->incrementJailTurns();
        }
        for (std::size_t j = 0; j < saveData.players[i].skillCards.size(); ++j) {
            SkillCard* card = GameManagerInternal::cloneSkillCardByName(saveData.players[i].skillCards[j].cardName);
            card->setValue(saveData.players[i].skillCards[j].value);
            card->setRemainingDuration(saveData.players[i].skillCards[j].remainingDuration);
            player->getInventory().addSkillCard(card);
        }
        players.push_back(player);
    }

    std::vector<Property*> props = board.getAllProperties();
    for (std::size_t i = 0; i < props.size(); ++i) {
        props[i]->clearOwner();
        props[i]->setStatus(PropertyStatus::BANK);
        props[i]->clearFestival();
        Street* s = dynamic_cast<Street*>(props[i]);
        if (s != 0) s->setBuildingLevel(0);
    }

    for (std::size_t i = 0; i < saveData.properties.size(); ++i) {
        Property* prop = board.getPropertyByCode(saveData.properties[i].code);
        if (prop == 0) continue;
        Player* owner = saveData.properties[i].ownerName != "BANK" ? findPlayerByName(saveData.properties[i].ownerName) : 0;
        if (owner != 0) {
            prop->setOwner(owner);
            owner->addProperty(prop);
            prop->setStatus(saveData.properties[i].status);
        }
        int steps = 0;
        for (int multiplier = saveData.properties[i].festivalMultiplier; multiplier > 1; multiplier /= 2) {
            ++steps;
        }
        prop->restoreFestivalState(steps, saveData.properties[i].festivalTurnsRemaining);
        Street* s = dynamic_cast<Street*>(prop);
        if (s != 0) s->setBuildingLevel(saveData.properties[i].buildingLevel);
    }

    skillDeck.clear();
    for (std::size_t i = 0; i < saveData.skillDeckState.skillDeckCardNames.size(); ++i) {
        skillDeck.addCard(GameManagerInternal::cloneSkillCardByName(saveData.skillDeckState.skillDeckCardNames[i]));
    }
}

Player& GameManager::getCurrentPlayer() { return *players[static_cast<std::size_t>(currentPlayerIndex)]; }
const std::vector<Player*>& GameManager::getPlayers() const { return players; }
Board& GameManager::getBoard() { return board; }
const Board& GameManager::getBoard() const { return board; }
Bank& GameManager::getBank() { return bank; }
const Bank& GameManager::getBank() const { return bank; }
TransactionLogger& GameManager::getLogger() { return logger; }
const TransactionLogger& GameManager::getLogger() const { return logger; }
const GameConfig& GameManager::getGameConfig() const { return gameConfig; }
const CardDeck<SkillCard>& GameManager::getSkillDeck() const { return skillDeck; }
const std::vector<SkillCard*>& GameManager::getDiscardedSkillCards() const { return discardedSkillCards; }
bool GameManager::getHasRolledThisTurn() const { return hasRolledThisTurn; }
bool GameManager::getHasUsedSkillThisTurn() const { return hasUsedSkillThisTurn; }
TurnPhase GameManager::getTurnPhase() const { return turnPhase; }
bool GameManager::getCurrentRollWasDouble() const { return currentRollWasDouble; }
void GameManager::setTurnStateForTesting(TurnPhase phase, bool shouldEnd, bool rollWasDouble) {
    turnPhase = phase;
    turnShouldEnd = shouldEnd;
    currentRollWasDouble = rollWasDouble;
}
