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

#include <cstdio>
#include "log.hpp"
#include "client.hpp"
#include "xmlconf.hpp"
#include "pngtexdb.hpp"
#include "fontexdb.hpp"
#include "mapbindb.hpp"
#include "emoticondb.hpp"
#include "notifyboard.hpp"
#include "pngtexoffdb.hpp"
#include "clientargparser.hpp"

// global variables, decide to follow pattern in MapEditor
// put all global in one place and create them togother

ClientArgParser *g_ClientArgParser = nullptr;
Log             *g_Log             = nullptr; // log information handler, must be inited first
PNGTexDB        *g_ProgUseDB       = nullptr; // database for all PNG texture only
PNGTexDB        *g_GroundItemDB    = nullptr; // database for all PNG texture only
PNGTexDB        *g_CommonItemDB    = nullptr; // database for all PNG texture only
PNGTexDB        *g_MapDB           = nullptr;
PNGTexOffDB     *g_HeroDB          = nullptr; // database for hero
PNGTexOffDB     *g_MonsterDB       = nullptr; // database for monster
PNGTexOffDB     *g_WeaponDB        = nullptr; // database for weapon
PNGTexOffDB     *g_MagicDB         = nullptr; // database for magic
EmoticonDB      *g_EmoticonDB      = nullptr; // database for emoticons
MapBinDB        *g_MapBinDB        = nullptr;
FontexDB        *g_FontexDB        = nullptr;
XMLConf         *g_XMLConf         = nullptr; // for client configure XML parsing
SDLDevice       *g_SDLDevice       = nullptr; // for SDL hardware device
NotifyBoard     *g_NotifyBoard     = nullptr;
Client          *g_Client          = nullptr; // gobal instance

int main(int argc, char *argv[])
{
    std::srand((unsigned int)std::time(nullptr));
    try{
        arg_parser stCmdParser(argc, argv);
        auto fnAtExit = []()
        {
            delete g_ClientArgParser; g_ClientArgParser = nullptr;
            delete g_Log            ; g_Log             = nullptr;
            delete g_XMLConf        ; g_XMLConf         = nullptr;
            delete g_SDLDevice      ; g_SDLDevice       = nullptr;
            delete g_ProgUseDB      ; g_ProgUseDB       = nullptr;
            delete g_GroundItemDB   ; g_GroundItemDB    = nullptr;
            delete g_CommonItemDB   ; g_CommonItemDB    = nullptr;
            delete g_MapDB          ; g_MapDB           = nullptr;
            delete g_HeroDB         ; g_HeroDB          = nullptr;
            delete g_MonsterDB      ; g_MonsterDB       = nullptr;
            delete g_FontexDB       ; g_FontexDB        = nullptr;
            delete g_MapBinDB       ; g_MapBinDB        = nullptr;
            delete g_EmoticonDB     ; g_EmoticonDB      = nullptr;
            delete g_NotifyBoard    ; g_NotifyBoard     = nullptr;
            delete g_Client         ; g_Client          = nullptr;
        };

        std::atexit(fnAtExit);

        g_ClientArgParser = new ClientArgParser(stCmdParser);
        g_Log             = new Log("mir2x-client-v0.1");

    }catch(const std::exception &e){
        std::fprintf(stderr, "Caught exception: %s\n", e.what());
        return -1;
    }catch(...){
        std::fprintf(stderr, "Caught unknown exception, exit...\n");
        return -1;
    }

    try{
        g_XMLConf         = new XMLConf();
        g_SDLDevice       = new SDLDevice();
        g_ProgUseDB       = new PNGTexDB(1024);
        g_GroundItemDB    = new PNGTexDB(1024);
        g_CommonItemDB    = new PNGTexDB(1024);
        g_MapDB           = new PNGTexDB(8192);
        g_HeroDB          = new PNGTexOffDB(1024);
        g_MonsterDB       = new PNGTexOffDB(1024);
        g_WeaponDB        = new PNGTexOffDB(1024);
        g_MagicDB         = new PNGTexOffDB(1024);
        g_FontexDB        = new FontexDB(1024);
        g_MapBinDB        = new MapBinDB();
        g_EmoticonDB      = new EmoticonDB();
        g_Client          = new Client();       // loads fontex resource
        g_NotifyBoard     = new NotifyBoard();  // needs fontex

        g_Client->MainLoop();

    }catch(const std::exception &e){
        g_Log->AddLog(LOGTYPE_FATAL, "Caught exception: %s", e.what());
    }catch(...){
        g_Log->AddLog(LOGTYPE_FATAL, "Caught unknown exception, exit...");
    }
    return 0;
}
