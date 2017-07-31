/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 07/31/2017 01:44:57
 *
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */
#include "log.hpp"
#include "game.hpp"
#include "xmlconf.hpp"
#include "clientenv.hpp"
#include "pngtexdbn.hpp"
#include "fontexdbn.hpp"
#include "emoticondbn.hpp"
#include "pngtexoffdbn.hpp"

// global variables, decide to follow pattern in MapEditor
// put all global in one place and create them togother

Log            *g_Log           = nullptr; // log information handler, must be inited first
ClientEnv      *g_ClientEnv     = nullptr;
PNGTexDBN      *g_ProgUseDBN    = nullptr; // database for all PNG texture only
PNGTexDBN      *g_GroundItemDBN = nullptr; // database for all PNG texture only
PNGTexDBN      *g_MapDBN        = nullptr;
PNGTexOffDBN   *g_HeroDBN       = nullptr; // database for hero
PNGTexOffDBN   *g_MonsterDBN    = nullptr; // database for monster
PNGTexOffDBN   *g_WeaponDBN     = nullptr; // database for weapon
EmoticonDBN    *g_EmoticonDBN   = nullptr; // database for emoticons
FontexDBN      *g_FontexDBN     = nullptr;
XMLConf        *g_XMLConf       = nullptr; // for game configure XML parsing
SDLDevice      *g_SDLDevice     = nullptr; // for SDL hardware device
Game           *g_Game          = nullptr; // gobal instance

int main()
{
    std::srand((unsigned int)std::time(nullptr));

    auto fnAtExit = []() -> void
    {
        delete g_Log           ; g_Log           = nullptr;
        delete g_ClientEnv     ; g_ClientEnv     = nullptr;
        delete g_XMLConf       ; g_XMLConf       = nullptr;
        delete g_SDLDevice     ; g_SDLDevice     = nullptr;
        delete g_ProgUseDBN    ; g_ProgUseDBN    = nullptr;
        delete g_GroundItemDBN ; g_GroundItemDBN = nullptr;
        delete g_MapDBN        ; g_MapDBN        = nullptr;
        delete g_HeroDBN       ; g_HeroDBN       = nullptr;
        delete g_MonsterDBN    ; g_MonsterDBN    = nullptr;
        delete g_FontexDBN     ; g_FontexDBN     = nullptr;
        delete g_EmoticonDBN   ; g_EmoticonDBN   = nullptr;
        delete g_Game          ; g_Game          = nullptr;
    };

    std::atexit(fnAtExit);

    g_Log           = new Log("mir2x-client-v0.1");
    g_ClientEnv     = new ClientEnv();
    g_XMLConf       = new XMLConf();
    g_SDLDevice     = new SDLDevice();
    g_ProgUseDBN    = new PNGTexDBN();
    g_GroundItemDBN = new PNGTexDBN();
    g_MapDBN        = new PNGTexDBN();
    g_HeroDBN       = new PNGTexOffDBN();
    g_MonsterDBN    = new PNGTexOffDBN();
    g_WeaponDBN     = new PNGTexOffDBN();
    g_FontexDBN     = new FontexDBN();
    g_EmoticonDBN   = new EmoticonDBN();
    g_Game          = new Game();

    g_Game->MainLoop();
    return 0;
}
