#ifndef ACTION_HPP
#define ACTION_HPP
#include "Tile.hpp"
#include "GameTypes.hpp"

class Action : public Tile
{
private:
    ActionType actionType;
    int fixedAmount;

public:
    Action();
    Action(int id, const string &name, const string &type, ActionType actiontype, int fixedAmount = 0);
    ActionType getActionType() const;
    int getFixedAmount() const;
};

#endif