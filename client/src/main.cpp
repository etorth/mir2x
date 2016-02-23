#include "game.hpp"

int main(int argc, char* argv[])
{
    Game *pGame = new Game();

    pGame->Init();
    pGame->MainLoop();

    delete pGame;
    return 0;
}
