#include "core/foundation/Dice.hpp"
#include <cstdlib>

Dice::Dice(int sides) : sides(sides) {}

int Dice::roll() const
{
    return (std::rand() % sides) + 1;
}

std::pair<int, int> Dice::rollPair() const
{
    return std::make_pair(roll(), roll());
}
