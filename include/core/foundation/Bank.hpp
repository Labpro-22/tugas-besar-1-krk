#pragma once

class Player;

class Bank
{
public:
    Bank() = default;

    void payPlayer(Player &player, int amount);
    void collectFromPlayer(Player &player, int amount);
    bool canPlayerAfford(const Player &player, int amount) const;
};
