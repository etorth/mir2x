/*
 * =====================================================================================
 *
 *       Filename: net.cpp
 *        Created: 02/23/2016 00:09:59
 *  Last Modified: 03/14/2016 23:18:35
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

void Game::RunASIO()
{
    // this function will run in another thread
    // make sure there is no data race

    m_NetIO.RunIO([this](){ ReadHC(); });
}

void Game::ReadHC()
{
    static std::function<void(uint8_t)> fnProcessHC = [&](uint8_t nSMID){
        switch(nSMID){
            case SM_PING:           OnPing()   ; break;
            case SM_LOGINOK:        OnLoginOK(); break;
            default: break;
        }
        m_NetIO.ReadHC(fnProcessHC);
    };
    m_NetIO.ReadHC(fnProcessHC);
}

void Game::OnPing()
{

}

void Game::OnLoginOK()
{

}
