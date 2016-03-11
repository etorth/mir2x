#include "game.hpp"

int main()
{
    Game *pGame = new Game();

    pGame->Init();
    pGame->MainLoop();

    delete pGame;
    return 0;
}
