#include "core/foundation/Bank.hpp"
#include "models/player/Player.hpp"

void Bank::payPlayer(Player &player, int amount)
{
    if (amount > 0)
    {
        player.addMoney(amount);
    }
}

void Bank::collectFromPlayer(Player &player, int amount)
{
    if (amount > 0)
    {
        player.deductMoney(amount);
    }
}

bool Bank::canPlayerAfford(const Player &player, int amount) const
{
    return player.getMoney() >= amount;
}
