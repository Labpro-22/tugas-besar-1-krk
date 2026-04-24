#pragma once

#include <cstddef>

class GameManager;
class Property;

class AuctionSession {
private:
    GameManager& game;

public:
    explicit AuctionSession(GameManager& game);

    bool run(Property& property, std::size_t startingPlayerIndex);
};
