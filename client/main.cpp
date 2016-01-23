#include "game.hpp"

int main(int argc, char* argv[])
{
    Game::StartSystem();

    Game stGame;
    stGame.Init();
    stGame.MainLoop();
    return 0;
}
