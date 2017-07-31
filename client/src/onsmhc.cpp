/*
 * =====================================================================================
 *
 *       Filename: onsmhc.cpp
 *        Created: 02/23/2016 00:09:59
 *  Last Modified: 07/30/2017 20:05:35
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

void Game::OnServerMessage(uint8_t nHC, const uint8_t *pData, size_t nDataLen)
{
    // 1. update the time when last message received
    m_NetPackTick = GetTimeTick();

    // 2. handle messages
    switch(nHC){
        case SM_UPDATEHP:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_UPDATEHP(pData, nDataLen);
                }
                break;
            }
        case SM_DEADFADEOUT:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_DEADFADEOUT(pData, nDataLen);
                }
                break;
            }
        case SM_EXP:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_EXP(pData, nDataLen);
                }
                break;
            }

        case SM_SHOWDROPITEM:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_SHOWDROPITEM(pData, nDataLen);
                }
                break;
            }
        case SM_PING     : Net_PING     (pData, nDataLen); break;
        case SM_LOGINOK  : Net_LOGINOK  (pData, nDataLen); break;
        case SM_CORECORD : Net_CORECORD (pData, nDataLen); break;
        case SM_LOGINFAIL: Net_LOGINFAIL(pData, nDataLen); break;
        case SM_ACTION   : Net_ACTION   (pData, nDataLen); break;
        default          :                                 break;
    }
}

void Game::Net_PING(const uint8_t *, size_t)
{
}

void Game::Net_LOGINOK(const uint8_t *pData, size_t nDataLen)
{
    SwitchProcess(m_CurrentProcess->ID(), PROCESSID_RUN);
    if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
        pRun->Net_LOGINOK(pData, nDataLen);
    }else{
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_INFO, "failed to jump into main loop");
    }
}

void Game::Net_LOGINFAIL(const uint8_t *pData, size_t)
{
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_WARNING, "login failed: ID = %d", (int)(((SMLoginFail *)(pData))->FailID));
}

void Game::Net_ACTION(const uint8_t *pData, size_t nDataLen)
{
    if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
        pRun->Net_ACTION(pData, nDataLen);
    }
}

void Game::Net_CORECORD(const uint8_t *pData, size_t nDataLen)
{
    if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
        pRun->Net_CORECORD(pData, nDataLen);
    }
}
