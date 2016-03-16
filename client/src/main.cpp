#include "game.hpp"

// global variables, decide to follow pattern in MapEditor
// put all global in one place and create them togother

Log         *g_Log         = nullptr; // log information handler, must be inited first
PNGTexDB    *g_PNGTexDB    = nullptr; // database for all PNG texture only
PNGTexOffDB *g_PNGTexOffDB = nullptr; // database for all PNG texture and offset information
XMLExt      *g_Conf        = nullptr; // for game configure XML parsing
SDLDevice   *g_SDLDevice   = nullptr; // for SDL hardware device
Game        *g_Game        = nullptr; // gobal instance

int main()
{
    g_GameLog = new GameLog("./mir2x.log");
    g_Game->Init();
    g_Game->MainLoop();

    delete pGame;
    return 0;
}
