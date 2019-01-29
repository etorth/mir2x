/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 08/31/2015 08:52:57 PM
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
#include "client.hpp"
#include "xmlconf.hpp"
#include "pngtexdbn.hpp"
#include "fontexdbn.hpp"
#include "mapbindbn.hpp"
#include "emoticondbn.hpp"
#include "notifyboard.hpp"
#include "pngtexoffdbn.hpp"
#include "clientargparser.hpp"

// global variables, decide to follow pattern in MapEditor
// put all global in one place and create them togother

ClientArgParser *g_ClientArgParser = nullptr;
Log             *g_Log             = nullptr; // log information handler, must be inited first
PNGTexDBN       *g_ProgUseDBN      = nullptr; // database for all PNG texture only
PNGTexDBN       *g_GroundItemDBN   = nullptr; // database for all PNG texture only
PNGTexDBN       *g_CommonItemDBN   = nullptr; // database for all PNG texture only
PNGTexDBN       *g_MapDBN          = nullptr;
PNGTexOffDBN    *g_HeroDBN         = nullptr; // database for hero
PNGTexOffDBN    *g_MonsterDBN      = nullptr; // database for monster
PNGTexOffDBN    *g_WeaponDBN       = nullptr; // database for weapon
PNGTexOffDBN    *g_MagicDBN        = nullptr; // database for magic
EmoticonDBN     *g_EmoticonDBN     = nullptr; // database for emoticons
MapBinDBN       *g_MapBinDBN       = nullptr;
FontexDBN       *g_FontexDBN       = nullptr;
XMLConf         *g_XMLConf         = nullptr; // for client configure XML parsing
SDLDevice       *g_SDLDevice       = nullptr; // for SDL hardware device
NotifyBoard     *g_NotifyBoard     = nullptr;
Client          *g_Client          = nullptr; // gobal instance

int main(int argc, char *argv[])
{
    std::srand((unsigned int)std::time(nullptr));
    arg_parser stCmdParser(argc, argv);

    auto fnAtExit = []()
    {
        delete g_ClientArgParser; g_ClientArgParser = nullptr;
        delete g_Log            ; g_Log             = nullptr;
        delete g_XMLConf        ; g_XMLConf         = nullptr;
        delete g_SDLDevice      ; g_SDLDevice       = nullptr;
        delete g_ProgUseDBN     ; g_ProgUseDBN      = nullptr;
        delete g_GroundItemDBN  ; g_GroundItemDBN   = nullptr;
        delete g_CommonItemDBN  ; g_CommonItemDBN   = nullptr;
        delete g_MapDBN         ; g_MapDBN          = nullptr;
        delete g_HeroDBN        ; g_HeroDBN         = nullptr;
        delete g_MonsterDBN     ; g_MonsterDBN      = nullptr;
        delete g_FontexDBN      ; g_FontexDBN       = nullptr;
        delete g_MapBinDBN      ; g_MapBinDBN       = nullptr;
        delete g_EmoticonDBN    ; g_EmoticonDBN     = nullptr;
        delete g_NotifyBoard    ; g_NotifyBoard     = nullptr;
        delete g_Client         ; g_Client            = nullptr;
    };

    std::atexit(fnAtExit);

    g_ClientArgParser = new ClientArgParser(stCmdParser);
    g_Log             = new Log("mir2x-client-v0.1");
    g_XMLConf         = new XMLConf();
    g_SDLDevice       = new SDLDevice();
    g_ProgUseDBN      = new PNGTexDBN();
    g_GroundItemDBN   = new PNGTexDBN();
    g_CommonItemDBN   = new PNGTexDBN();
    g_MapDBN          = new PNGTexDBN();
    g_HeroDBN         = new PNGTexOffDBN();
    g_MonsterDBN      = new PNGTexOffDBN();
    g_WeaponDBN       = new PNGTexOffDBN();
    g_MagicDBN        = new PNGTexOffDBN();
    g_FontexDBN       = new FontexDBN();
    g_MapBinDBN       = new MapBinDBN();
    g_EmoticonDBN     = new EmoticonDBN();
    g_Client          = new Client();       // loads fontex resource
    g_NotifyBoard     = new NotifyBoard();  // needs fontex

    try{
        g_Client->MainLoop();
    }catch(const std::exception &e){
        g_Log->AddLog(LOGTYPE_FATAL, "Caught exception: %s", e.what());
    }catch(...){
        g_Log->AddLog(LOGTYPE_FATAL, "Caught unknown exception, exit...");
    }
    return 0;
}
