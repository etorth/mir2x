/*
 * =====================================================================================
 *
 *       Filename: processrunnet.cpp
 *        Created: 08/31/2015 03:43:46 AM
 *  Last Modified: 03/27/2017 14:16:06
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
    m_MyHero = new MyHero(stSMLOK.GUID, stSMLOK.UID, stSMLOK.AddTime, (bool)(stSMLOK.Male));

    m_MyHero->ResetLocation(stSMLOK.MapID, (int)(stSMLOK.X), (int)(stSMLOK.Y));
    m_MyHero->ResetDirection((int)(stSMLOK.Direction));
    m_MyHero->ResetLevel((int)(stSMLOK.Level));
    m_MyHero->ResetJob((int)(stSMLOK.JobID));
}

void ProcessRun::Net_ACTIONSTATE(const uint8_t *pBuf, size_t)
{
    SMActionState stSMAS = *((const SMActionState *)pBuf);
    if(stSMAS.MapID != m_MapID){ return; }

    auto pRecord = m_CreatureRecord.find(((uint64_t)stSMAS.UID << 32) + stSMAS.AddTime);
    if(true
            && pRecord != m_CreatureRecord.end()
            && pRecord->second
            && pRecord->second->MapID() == stSMAS.MapID){
        auto pCreature = pRecord->second;

        pCreature->ResetLocation(stSMAS.MapID, stSMAS.X, stSMAS.Y);

        pCreature->ResetAction((int)stSMAS.Action);
        pCreature->ResetDirection((int)stSMAS.Direction);

        pCreature->ResetSpeed((int)stSMAS.Speed);
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

    Creature *pCreature = nullptr;
    uint64_t nCOKey = ((((uint64_t)stSMCOR.Common.UID) << 32) + stSMCOR.Common.AddTime);

    auto pRecord = m_CreatureRecord.find(nCOKey);
    if(pRecord == m_CreatureRecord.end()){
        switch(stSMCOR.Type){
            case CREATURE_MONSTER:
                {
                    pCreature = new Monster(stSMCOR.Monster.MonsterID, stSMCOR.Common.UID, stSMCOR.Common.AddTime);
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
        pCreature = pRecord->second;
    }

    if(pCreature){
        pCreature->ResetLocation(stSMCOR.Common.MapID, (int)stSMCOR.Common.MapX, (int)stSMCOR.Common.MapY);

        pCreature->ResetAction((int)stSMCOR.Common.Action);
        pCreature->ResetDirection((int)stSMCOR.Common.Direction);

        pCreature->ResetSpeed((int)stSMCOR.Common.Speed);

        m_CreatureRecord[nCOKey] = pCreature;
    }
}
