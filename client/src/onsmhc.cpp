/*
 * =====================================================================================
 *
 *       Filename: onsmhc.cpp
 *        Created: 02/23/2016 00:09:59
 *  Last Modified: 06/14/2016 00:25:11
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
        case SM_MOTIONSTATE:    Net_MOTIONSTATE();  break;
        case SM_MONSTERGINFO:   Net_MONSTERGINFO(); break;
        default: break;
    }

    m_NetIO.ReadHC([&](uint8_t nHC){ OperateHC(nHC); });
}

void Game::Net_PING()
{
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "on ping");
}

void Game::Net_LOGINOK()
{
    auto fnDoLoginOK = [this](const uint8_t *pBuf, size_t nLen){
        SwitchProcess(m_CurrentProcess->ID(), PROCESSID_RUN);
        auto pRun = (ProcessRun *)m_CurrentProcess;

        extern Log *g_Log;
        if(pRun){
            pRun->Net_LOGINOK(pBuf, nLen);
            g_Log->AddLog(LOGTYPE_INFO, "login succeed");
        }else{
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

void Game::Net_MOTIONSTATE()
{
    auto fnMotionState = [this](const uint8_t *pBuf, size_t nLen){
        // 1. receive object motion state update while game is not in running state
        //    do we need to inform server?
        if(!(m_CurrentProcess && m_CurrentProcess->ID() == PROCESSID_RUN)){ return; }

        // 2. ok we are in running state
        auto pRun = (ProcessRun *)m_CurrentProcess;
        pRun->Net_MOTIONSTATE(pBuf, nLen);
    };

    Read(sizeof(SMMotionState), fnMotionState);
}

void Game::Net_MONSTERGINFO()
{
    if(!ProcessValid(PROCESSID_RUN)){ return; }
    auto fnOnGetMonsterGInfo = [this](const uint8_t *pBuf, size_t nLen){
        if(!ProcessValid(PROCESSID_RUN)){ return; }
        ((ProcessRun *)m_CurrentProcess)->Net_MONSTERGINFO(pBuf, nLen);
    };

    Read(sizeof(SMMonsterGInfo), fnOnGetMonsterGInfo);
}

void Game::Net_CORECORD()
{
    if(!ProcessValid(PROCESSID_RUN)){ return; }
    auto fnOnGetCORecord = [this](const uint8_t *pBuf, size_t nLen){
        if(!ProcessValid(PROCESSID_RUN)){ return; }
        ((ProcessRun *)m_CurrentProcess)->Net_CORECORD(pBuf, nLen);
    };

    Read(sizeof(SMCORecord), fnOnGetCORecord);
}
