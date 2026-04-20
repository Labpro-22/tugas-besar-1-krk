#ifndef SPECIAL_HPP
#define SPECIAL_HPP
#include "GameTypes.hpp"
#include "Tile.hpp"
using namespace std;

class Special : public Tile
{
private:
    SpecialType specialType;

public:
    Special(int id, const string &name, const string &type, SpecialType specialType);

    SpecialType getSpecialType() const;
};

#endif
