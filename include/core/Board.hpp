#ifndef BOARD_HPP
#define BOARD_HPP

#include <iostream>
using namespace std;

class Property;
class Tile;

class Board
{
private:
    vector<Tile *> tiles;

public:
    Board();
    Board(const Board &);
    ~Board();
    void loadConfiguration();
    Tile *getTile();
    void displayBoard();
};

#endif

/*

*/