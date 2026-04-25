#pragma once
#include <utility>
using namespace std;
class Dice
{
private:
    int sides;

public:
    explicit Dice(int sides = 6);

    int roll() const;
    pair<int, int> rollPair() const;
};
