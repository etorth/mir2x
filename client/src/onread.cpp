/*
 * =====================================================================================
 *
 *       Filename: onread.cpp
 *        Created: 01/15/2016 09:04:57
 *  Last Modified: 01/15/2016 09:16:13
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

#include "game.hpp"

void Game::OnRead(uint8_t chMsgHead)
{
    switch(chMsgHead){
        case SM_NONE:       OnReadNone();          break;
        case SM_LOGINOK:    OnReadLoginOK();       break;
        case SM_LOGINERROR: OnReadLoginError();    break;
        case SM_PING:       OnReadPing();          break;
        default: break;
    }

    asio::asyn_read();
}

void Game::OnReadNone()
{
    m_PackTick = SDL_GetTicks();
}

void Game::OnReadNone()
