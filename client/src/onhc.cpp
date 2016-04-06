/*
 * =====================================================================================
 *
 *       Filename: onhc.cpp
 *        Created: 02/23/2016 00:09:59
 *  Last Modified: 04/06/2016 02:26:10
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
#include "message.hpp"

void Game::OperateHC(uint8_t nHC)
{
    switch(nHC){
        case SM_PING:           OnPing();      break;
        case SM_LOGINOK:        OnLoginOK();   break;
        case SM_LOGINFAIL:      OnLoginFail(); break;
        default: break;
    }

    m_NetIO.ReadHC([&](uint8_t nHC){ OperateHC(nHC); });
}

void Game::OnPing()
{
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "on ping");
}

void Game::OnLoginOK()
{
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "login succeed");
}

void Game::OnLoginFail()
{
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "login failed");
}
