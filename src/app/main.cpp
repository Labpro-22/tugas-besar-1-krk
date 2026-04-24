#include <cstdlib>
#include <ctime>
#include <iostream>
#include "core/game_manager/GameManager.hpp"
#include "utils/io/CLIHandler.hpp"

int main()
{
    std::srand(static_cast<unsigned int>(std::time(0)));
    try
    {
        CLIHandler cli;
        GameManager *game = GameManager::getInstance();
        game->setDisplayHandler(&cli);
        game->start();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
