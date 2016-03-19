/*
 * =====================================================================================
 *
 *       Filename: net.cpp
 *        Created: 02/23/2016 00:09:59
 *  Last Modified: 03/19/2016 02:57:46
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
#include "xmlconf.hpp"
#include "message.hpp"

void Game::RunASIO()
{
    // this function will run in another thread
    // make sure there is no data race

    // TODO
    // may need lock here since g_XMLConf may used in main thread also
    extern XMLConf *g_XMLConf;
    m_NetIO.RunIO(
            g_XMLConf->GetXMLNode("/Root/Network/Server/IP"  )->GetText(),
            g_XMLConf->GetXMLNode("/Root/Network/Server/Port")->GetText(),
            [this](uint8_t nHC){ OperateHC(nHC); });
}

void Game::OperateHC(uint8_t nHC)
{
    switch(nHC){
        case SM_PING:           OnPing()   ; break;
        case SM_LOGINOK:        OnLoginOK(); break;
        default: break;
    }

    m_NetIO.ReadHC([&](uint8_t nHC){ OperateHC(nHC); });
}

void Game::OnPing()
{

}

void Game::OnLoginOK()
{

}
