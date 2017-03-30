/*
 * =====================================================================================
 *
 *       Filename: processrunnet.cpp
 *        Created: 08/31/2015 03:43:46 AM
 *  Last Modified: 03/29/2017 16:53:58
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
    m_MyHero = new MyHero(stSMLOK.GUID, stSMLOK.UID, (bool)(stSMLOK.Male), this);

    m_MyHero->ResetLocation((int)(stSMLOK.X), (int)(stSMLOK.Y));
    m_MyHero->ResetDirection((int)(stSMLOK.Direction));
    m_MyHero->ResetLevel((int)(stSMLOK.Level));
    m_MyHero->ResetJob((int)(stSMLOK.JobID));
}

void ProcessRun::Net_ACTIONSTATE(const uint8_t *pBuf, size_t)
{
    SMActionState stSMAS;
    std::memcpy(&stSMAS, pBuf, sizeof(stSMAS));

    if(stSMAS.MapID == m_MapID){
        auto pRecord = m_CreatureRecord.find(stSMAS.UID);
        if(pRecord != m_CreatureRecord.end()){
            if(auto pCreature = pRecord->second){
                pCreature->OnActionState((int)(stSMAS.Action), (int)(stSMAS.Direction), stSMAS.Speed, stSMAS.X, stSMAS.Y);
            }
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
        int nX = stSMCOR.Common.MapX;
        int nY = stSMCOR.Common.MapY;

        int nSpeed     = stSMCOR.Common.Speed;
        int nAction    = stSMCOR.Common.Action;
        int nDirection = stSMCOR.Common.Direction;

        auto pRecord = m_CreatureRecord.find(stSMCOR.Common.UID);
        if(pRecord == m_CreatureRecord.end()){
            switch(stSMCOR.Type){
                case CREATURE_MONSTER:
                    {
                        auto pMonster = new Monster(stSMCOR.Monster.MonsterID, stSMCOR.Common.UID, this);

                        pMonster->ResetSpeed(nSpeed);
                        pMonster->ResetAction(nAction);
                        pMonster->ResetLocation(nX, nY);
                        pMonster->ResetDirection(nDirection);

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
                pRecord->second->OnCORecord(nAction, nDirection, nSpeed, nX, nY);
            }
        }
    }
}
