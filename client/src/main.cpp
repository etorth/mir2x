#include "game.hpp"
#include "log.hpp"
#include "xmlconf.hpp"
#include "pngtexdbn.hpp"
#include "pngtexoffdbn.hpp"

// global variables, decide to follow pattern in MapEditor
// put all global in one place and create them togother

Log            *g_Log           = nullptr; // log information handler, must be inited first
PNGTexDBN      *g_PNGTexDBN     = nullptr; // database for all PNG texture only
PNGTexOffDBN   *g_PNGTexOffDBN  = nullptr; // database for all PNG texture and offset information
XMLConf        *g_XMLConf       = nullptr; // for game configure XML parsing
SDLDevice      *g_SDLDevice     = nullptr; // for SDL hardware device
Game           *g_Game          = nullptr; // gobal instance

int main()
{
    auto fnAtExit = [](){
        delete g_Log;  g_Log  = nullptr;
        delete g_Game; g_Game = nullptr;
    };

    std::atexit(fnAtExit);

    g_Log = new Log();



    g_Game->Init();
    g_Game->MainLoop();

    return 0;
}
