#ifndef TILE_HPP
#define TILE_HPP
#include <string>
#include <vector>
#include <iostream>
using namespace std;

class Tile
{
protected:
    int id;
    string name;
    string type;

public:
    Tile();
    Tile(int id, const string &name, const string &type);
    int getTileId() const;
    string getTileName() const;
    string getTileType() const;
    ~Tile();
};

#endif