/*
 * =====================================================================================
 *
 *       Filename: processrunnet.cpp
 *        Created: 08/31/2015 03:43:46 AM
 *  Last Modified: 05/28/2017 00:33:20
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

#include <memory>
#include <cstring>

#include "monster.hpp"
#include "sysconst.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"

// we get all needed initialization info for init the process run
void ProcessRun::Net_LOGINOK(const uint8_t *pBuf, size_t nLen)
{
    if(pBuf && nLen && (nLen == sizeof(SMLoginOK))){
        SMLoginOK stSMLOK;
        std::memcpy(&stSMLOK, pBuf, nLen);

        LoadMap(stSMLOK.MapID);
        m_MyHero = new MyHero(stSMLOK.UID, stSMLOK.DBID, (bool)(stSMLOK.Male), 0, this, 
                {
                    ACTION_STAND,
                    0,
                    stSMLOK.Direction,
                    stSMLOK.X,
                    stSMLOK.Y,
                    stSMLOK.MapID
                });

        m_CreatureRecord[m_MyHero->UID()] = m_MyHero;

        {
            extern SDLDevice *g_SDLDevice;
            m_ViewX = m_MyHero->X() * SYS_MAPGRIDXP - g_SDLDevice->WindowW(false) / 2;
            m_ViewY = m_MyHero->Y() * SYS_MAPGRIDYP - g_SDLDevice->WindowH(false) / 2;
        }
    }
}

void ProcessRun::Net_ACTION(const uint8_t *pBuf, size_t)
{
    SMAction stSMA;
    std::memcpy(&stSMA, pBuf, sizeof(stSMA));

    ActionNode stAction
    {
        stSMA.Action,
        stSMA.ActionParam,
        stSMA.Speed,
        stSMA.Direction,
        stSMA.X,
        stSMA.Y,
        stSMA.EndX,
        stSMA.EndY,
        stSMA.MapID
    };

    if(stSMA.MapID == m_MapID){
        auto pRecord = m_CreatureRecord.find(stSMA.UID);
        if((pRecord != m_CreatureRecord.end()) && pRecord->second){
            pRecord->second->ParseNewAction(stAction, true);
        }
    }else{
        if(m_MyHero && m_MyHero->UID() == stSMA.UID){
            LoadMap(stSMA.MapID);

            auto nUID   = m_MyHero->UID();
            auto nDBID  = m_MyHero->DBID();
            auto bMale  = m_MyHero->Male();
            auto nDress = m_MyHero->DressID();

            for(auto pRecord: m_CreatureRecord){
                delete pRecord.second;
            }
            m_CreatureRecord.clear();

            m_MyHero = new MyHero(nUID, nDBID, bMale, nDress, this, stAction);
            m_CreatureRecord[m_MyHero->UID()] = m_MyHero;
        }
    }
}

void ProcessRun::Net_MONSTERGINFO(const uint8_t *pBuf, size_t)
{
    auto pInfo = (SMMonsterGInfo *)pBuf;
    Monster::GetGInfoRecord(pInfo->MonsterID).ResetLookID(pInfo->LookIDN, pInfo->LookID);
}

void ProcessRun::Net_CORECORD(const uint8_t *pBuf, size_t)
{
    SMCORecord stSMCOR;
    std::memcpy(&stSMCOR, pBuf, sizeof(stSMCOR));

    if(stSMCOR.Common.MapID == m_MapID){
        ActionNode stAction
        {
            stSMCOR.Common.Action,
            stSMCOR.Common.ActionParam,
            stSMCOR.Common.Speed,
            stSMCOR.Common.Direction,
            stSMCOR.Common.X,
            stSMCOR.Common.Y,
            stSMCOR.Common.EndX,
            stSMCOR.Common.EndY,
            stSMCOR.Common.MapID,
        };

        auto pRecord = m_CreatureRecord.find(stSMCOR.Common.UID);
        if(pRecord == m_CreatureRecord.end()){
            switch(stSMCOR.Type){
                case CREATURE_MONSTER:
                    {
                        if(auto pMonster = Monster::Create(stSMCOR.Common.UID, stSMCOR.Monster.MonsterID, this, stAction)){
                            m_CreatureRecord[stSMCOR.Common.UID] = pMonster;
                        }
                        break;
                    }
                case CREATURE_PLAYER:
                    {
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }else{
            if(pRecord->second){
                pRecord->second->ParseNewAction(stAction, true);
            }
        }
    }
}

void ProcessRun::Net_UPDATEHP(const uint8_t *pBuf, size_t)
{
    SMUpdateHP stSMUHP;
    std::memcpy(&stSMUHP, pBuf, sizeof(stSMUHP));

    if(stSMUHP.MapID == MapID()){
        auto pRecord = m_CreatureRecord.find(stSMUHP.UID);
        if((pRecord != m_CreatureRecord.end()) && pRecord->second){
            pRecord->second->UpdateHP(stSMUHP.HP, stSMUHP.HPMax);
        }
    }
}

void ProcessRun::Net_DEADFADEOUT(const uint8_t *pBuf, size_t)
{
    SMDeadFadeOut stSMDFO;
    std::memcpy(&stSMDFO, pBuf, sizeof(stSMDFO));

    if(stSMDFO.MapID == MapID()){
        auto pRecord = m_CreatureRecord.find(stSMDFO.UID);
        if((pRecord != m_CreatureRecord.end()) && pRecord->second){
            pRecord->second->DeadFadeOut();
        }
    }
}
