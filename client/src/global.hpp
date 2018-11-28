/*
 * =====================================================================================
 *
 *       Filename: global.hpp
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

#pragma once

#include "log.hpp"
#include "game.hpp"
#include "xmlconf.hpp"
#include "sdldevice.hpp"
#include "notifyboard.hpp"
#include "clientargparser.hpp"

#include "pngtexdbn.hpp"
#include "pngtexoffdbn.hpp"

#include "fontexdbn.hpp"
#include "mapbindbn.hpp"
#include "emoticondbn.hpp"

extern Log *g_Log;
extern Game *g_Game;
extern XMLConf *g_XMLConf;
extern SDLDevice *g_SDLDevice;

extern NotifyBoard *g_NotifyBoard;
extern ClientArgParser *g_ClientArgParser;

extern PNGTexDBN *g_MapDBN;
extern PNGTexDBN *g_ProgUseDBN;
extern PNGTexDBN *g_GroundItemDBN;
extern PNGTexDBN *g_CommonItemDBN;

extern PNGTexOffDBN *g_HeroDBN;
extern PNGTexOffDBN *g_MagicDBN;
extern PNGTexOffDBN *g_WeaponDBN;
extern PNGTexOffDBN *g_MonsterDBN;

extern FontexDBN *g_FontexDBN;
extern MapBinDBN *g_MapBinDBN;
extern EmoticonDBN *g_EmoticonDBN;
