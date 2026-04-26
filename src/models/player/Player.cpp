#include "models/player/Player.hpp"
#include <algorithm>
#include <stdexcept>
#include "models/base/Property.hpp"
#include "models/tiles/Street.hpp"

Player::Player(const std::string& name, int initialMoney)
    : name(name),
      money(initialMoney),
      position(1),
      status(PlayerStatus::ACTIVE),
      jailTurnsSpent(0),
      activeDiscountPercent(0),
      activeDiscountTurns(0),
      activeShieldTurns(0) {}

const std::string& Player::getName() const { return name; }
int Player::getMoney() const { return money; }
int Player::getPosition() const { return position; }
PlayerStatus Player::getStatus() const { return status; }
bool Player::isInJail() const { return status == PlayerStatus::JAILED; }
int Player::getJailTurnsSpent() const { return jailTurnsSpent; }
bool Player::isBankrupt() const { return status == PlayerStatus::BANKRUPT; }
bool Player::hasActiveShield() const { return activeShieldTurns > 0; }
int Player::getActiveDiscountPercent() const { return activeDiscountTurns > 0 ? activeDiscountPercent : 0; }
int Player::getActiveDiscountTurns() const { return activeDiscountTurns; }
int Player::getActiveShieldTurns() const { return activeShieldTurns; }

void Player::setPosition(int newPosition) { position = newPosition; }

void Player::move(int steps, int boardSize) {
    if (boardSize <= 0) {
        return;
    }
    int newPos = position + steps;
    while (newPos > boardSize) {
        newPos -= boardSize;
    }
    while (newPos <= 0) {
        newPos += boardSize;
    }
    position = newPos;
}

void Player::setStatus(PlayerStatus newStatus) { status = newStatus; }

void Player::sendToJail(int jailPosition) {
    position = jailPosition;
    status = PlayerStatus::JAILED;
    jailTurnsSpent = 0;
}

void Player::releaseFromJail() {
    status = PlayerStatus::ACTIVE;
    jailTurnsSpent = 0;
}

void Player::incrementJailTurns() { ++jailTurnsSpent; }
void Player::resetJailTurns() { jailTurnsSpent = 0; }

void Player::setBankrupt(bool value) {
    status = value ? PlayerStatus::BANKRUPT : PlayerStatus::ACTIVE;
}

void Player::activateDiscount(int percent, int turns) {
    if (turns < 0) turns = 0;
    activeDiscountPercent = percent < 0 ? 0 : percent;
    activeDiscountTurns = turns;
}

void Player::activateShield(int turns) {
    activeShieldTurns = turns < 0 ? 0 : turns;
}

void Player::clearActiveEffects() {
    activeDiscountPercent = 0;
    activeDiscountTurns = 0;
    activeShieldTurns = 0;
}

void Player::tickActiveEffects() {
    if (activeDiscountTurns > 0) {
        --activeDiscountTurns;
        if (activeDiscountTurns == 0) {
            activeDiscountPercent = 0;
        }
    }
    if (activeShieldTurns > 0) {
        --activeShieldTurns;
    }
}

Inventory& Player::getInventory() { return inventory; }
const Inventory& Player::getInventory() const { return inventory; }

void Player::addProperty(Property* property) {
    if (property == 0) return;
    if (std::find(ownedProperties.begin(), ownedProperties.end(), property) == ownedProperties.end()) {
        ownedProperties.push_back(property);
    }
}

void Player::removeProperty(Property* property) {
    std::vector<Property*>::iterator it = std::find(ownedProperties.begin(), ownedProperties.end(), property);
    if (it != ownedProperties.end()) {
        ownedProperties.erase(it);
    }
}

const std::vector<Property*>& Player::getOwnedProperties() const { return ownedProperties; }
int Player::countOwnedProperties() const { return static_cast<int>(ownedProperties.size()); }

int Player::countOwnedType(PropertyType type) const {
    int count = 0;
    for (std::size_t i = 0; i < ownedProperties.size(); ++i) {
        if (ownedProperties[i] != 0 && ownedProperties[i]->getPropertyType() == type) {
            ++count;
        }
    }
    return count;
}

int Player::countOwnedColorGroup(const std::string& colorGroup) const {
    int count = 0;
    for (std::size_t i = 0; i < ownedProperties.size(); ++i) {
        Street* street = dynamic_cast<Street*>(ownedProperties[i]);
        if (street != 0 && street->getColorGroup() == colorGroup) {
            ++count;
        }
    }
    return count;
}

int Player::countOwnedSkillCards() const {
    return static_cast<int>(inventory.getSkillCardCount());
}

int Player::getTotalWealth() const {
    int total = money;
    for (std::size_t i = 0; i < ownedProperties.size(); ++i) {
        if (ownedProperties[i] != 0) {
            total += ownedProperties[i]->getAssetValue();
        }
    }
    return total;
}

bool Player::canAfford(int amount) const { return money >= amount; }

void Player::addMoney(int amount) { money += amount; }

void Player::deductMoney(int amount) {
    money -= amount;
}

Player& Player::operator+=(int amount) {
    money += amount;
    return *this;
}

Player Player::operator+(int amount) const {
    return Player(name, money + amount);
}

bool Player::operator<(const Player& other) const {
    return getTotalWealth() < other.getTotalWealth();
}

bool Player::operator>(const Player& other) const {
    return getTotalWealth() > other.getTotalWealth();
}
