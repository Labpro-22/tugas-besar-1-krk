#pragma once
#include <string>
#include <vector>
#include "models/base/GameTypes.hpp"
#include "models/player/Inventory.hpp"

class Property;

class Player {
private:
    std::string name;
    int money;
    int position;
    PlayerStatus status;
    int jailTurnsSpent;
    int activeDiscountPercent;
    int activeDiscountTurns;
    int activeShieldTurns;
    Inventory inventory;
    std::vector<Property*> ownedProperties;

public:
    Player(const std::string& name, int initialMoney = 0);

    const std::string& getName() const;
    int getMoney() const;
    int getPosition() const;
    PlayerStatus getStatus() const;
    bool isInJail() const;
    int getJailTurnsSpent() const;
    bool isBankrupt() const;
    bool hasActiveShield() const;
    int getActiveDiscountPercent() const;
    int getActiveDiscountTurns() const;
    int getActiveShieldTurns() const;

    void setPosition(int newPosition);
    void move(int steps, int boardSize);
    void setStatus(PlayerStatus newStatus);
    void sendToJail(int jailPosition);
    void releaseFromJail();
    void incrementJailTurns();
    void resetJailTurns();
    void setBankrupt(bool value);
    void activateDiscount(int percent, int turns = 1);
    void activateShield(int turns = 1);
    void clearActiveEffects();
    void tickActiveEffects();

    Inventory& getInventory();
    const Inventory& getInventory() const;

    void addProperty(Property* property);
    void removeProperty(Property* property);
    const std::vector<Property*>& getOwnedProperties() const;

    int countOwnedProperties() const;
    int countOwnedType(PropertyType type) const;
    int countOwnedColorGroup(const std::string& colorGroup) const;
    int countOwnedSkillCards() const;
    int getTotalWealth() const;

    bool canAfford(int amount) const;
    void addMoney(int amount);
    void deductMoney(int amount);

    Player& operator+=(int amount);
    Player operator+(int amount) const;
    bool operator<(const Player& other) const;
    bool operator>(const Player& other) const;
};
