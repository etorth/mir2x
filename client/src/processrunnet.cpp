/*
 * =====================================================================================
 *
 *       Filename: processrunnet.cpp
 *        Created: 08/31/2015 03:43:46 AM
 *  Last Modified: 04/07/2017 13:08:43
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
#include "processrun.hpp"

// we get all needed initialization info for init the process run
void ProcessRun::Net_LOGINOK(const uint8_t *pBuf, size_t nLen)
{
    if(!(pBuf && nLen && nLen == sizeof(SMLoginOK))){ return; }

    SMLoginOK stSMLOK;
    std::memcpy(&stSMLOK, pBuf, nLen);

    LoadMap(stSMLOK.MapID);
    m_MyHero = new MyHero(stSMLOK.UID, stSMLOK.DBID, (bool)(stSMLOK.Male), 0, this);
    m_MyHero->ParseNewAction({ACTION_STAND, 0, 0, DIR_UP, 0, 0, 0, 0});
}

void ProcessRun::Net_ACTION(const uint8_t *pBuf, size_t)
{
    SMAction stSMA;
    std::memcpy(&stSMA, pBuf, sizeof(stSMA));

    if(stSMA.MapID == m_MapID){
        auto pRecord = m_CreatureRecord.find(stSMA.UID);
        if((pRecord != m_CreatureRecord.end()) && pRecord->second){
            ActionNode stAction;
            stAction.Action      = stSMA.Action;
            stAction.ActionParam = stSMA.ActionParam;
            stAction.Speed       = stSMA.Speed;
            stAction.Direction   = stSMA.Direction;
            stAction.X           = stSMA.X;
            stAction.Y           = stSMA.Y;
            stAction.EndX        = stSMA.EndX;
            stAction.EndY        = stSMA.EndY;

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
            stAction.Print();
#endif
            pRecord->second->ParseNewAction(stAction);
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

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
#endif

    if(stSMCOR.Common.MapID == m_MapID){
        ActionNode stAction;
        stAction.Action      = stSMCOR.Common.Action;
        stAction.ActionParam = stSMCOR.Common.ActionParam;
        stAction.X           = stSMCOR.Common.X;
        stAction.Y           = stSMCOR.Common.Y;
        stAction.EndX        = stSMCOR.Common.EndX;
        stAction.EndY        = stSMCOR.Common.EndY;
        stAction.Direction   = stSMCOR.Common.Direction;
        stAction.Speed       = stSMCOR.Common.Speed;

        auto pRecord = m_CreatureRecord.find(stSMCOR.Common.UID);
        if(pRecord == m_CreatureRecord.end()){
            switch(stSMCOR.Type){
                case CREATURE_MONSTER:
                    {
                        auto pMonster = new Monster(stSMCOR.Common.UID, stSMCOR.Monster.MonsterID, this);
                        pMonster->ParseNewAction(stAction);
                        m_CreatureRecord[stSMCOR.Common.UID] = pMonster;
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
                pRecord->second->ParseNewAction(stAction);
            }
        }
    }
}
