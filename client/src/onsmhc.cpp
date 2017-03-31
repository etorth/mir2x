/*
 * =====================================================================================
 *
 *       Filename: onsmhc.cpp
 *        Created: 02/23/2016 00:09:59
 *  Last Modified: 03/31/2017 13:21:22
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
#include "processrun.hpp"

void Game::OperateHC(uint8_t nHC)
{
    switch(nHC){
        case SM_PING:           Net_PING();         break;
        case SM_LOGINOK:        Net_LOGINOK();      break;
        case SM_CORECORD:       Net_CORECORD();     break;
        case SM_LOGINFAIL:      Net_LOGINFAIL();    break;
        case SM_ACTION:         Net_ACTION();       break;
        case SM_MONSTERGINFO:   Net_MONSTERGINFO(); break;
        default: break;
    }

    m_NetIO.ReadHC([this](uint8_t nHC){ OperateHC(nHC); });
    m_NetPackTick = GetTimeTick();
}

void Game::Net_PING()
{
    auto fnDoPing = [this](const uint8_t *, size_t){};
    Read(sizeof(SMPing), fnDoPing);
}

void Game::Net_LOGINOK()
{
    auto fnDoLoginOK = [this](const uint8_t *pBuf, size_t nLen){
        SwitchProcess(m_CurrentProcess->ID(), PROCESSID_RUN);
        if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
            pRun->Net_LOGINOK(pBuf, nLen);
        }else{
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_INFO, "failed to jump into main loop");
        }
    };

    Read(sizeof(SMLoginOK), fnDoLoginOK);
}

void Game::Net_LOGINFAIL()
{
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "login failed");
}

void Game::Net_ACTION()
{
    auto fnAction = [this](const uint8_t *pBuf, size_t nLen){
        if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
            pRun->Net_ACTION(pBuf, nLen);
        }
    };

    Read(sizeof(SMAction), fnAction);
}

void Game::Net_MONSTERGINFO()
{
    auto fnOnGetMonsterGInfo = [this](const uint8_t *pBuf, size_t nLen){
        if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
            pRun->Net_MONSTERGINFO(pBuf, nLen);
        }
    };

    Read(sizeof(SMMonsterGInfo), fnOnGetMonsterGInfo);
}

void Game::Net_CORECORD()
{
    auto fnOnGetCORecord = [this](const uint8_t *pBuf, size_t nLen){
        if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
            pRun->Net_CORECORD(pBuf, nLen);
        }
    };

    Read(sizeof(SMCORecord), fnOnGetCORecord);
}
