/*
 * =====================================================================================
 *
 *       Filename: processrunnet.cpp
 *        Created: 08/31/2015 03:43:46 AM
 *  Last Modified: 03/31/2017 00:52:09
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
    m_MyHero = new MyHero(stSMLOK.UID, stSMLOK.GUID, (bool)(stSMLOK.Male), this, 0, 0, 0, 0, 0);
}

void ProcessRun::Net_ACTIONSTATE(const uint8_t *pBuf, size_t)
{
    SMActionState stSMAS;
    std::memcpy(&stSMAS, pBuf, sizeof(stSMAS));

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    {
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_INFO, "ACTIONSTATE: X = %d, Y = %d, Action = %d, Direction = %d", stSMAS.X, stSMAS.Y, stSMAS.Action, stSMAS.Direction);
    }
#endif

    if(stSMAS.MapID == m_MapID){
        auto pRecord = m_CreatureRecord.find(stSMAS.UID);
        if(pRecord != m_CreatureRecord.end()){
            if(auto pCreature = pRecord->second){
                pCreature->OnReportAction((int)(stSMAS.Action), (int)(stSMAS.Direction), stSMAS.Speed, stSMAS.X, stSMAS.Y);
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

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    {
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_INFO,
                "CORECORD: X = %d, Y = %d, Action = %d, Direction = %d",
                stSMCOR.Common.MapX, stSMCOR.Common.MapY, stSMCOR.Common.Action, stSMCOR.Common.Direction);
    }
#endif

    if(stSMCOR.Common.MapID == m_MapID){
        int nX = stSMCOR.Common.MapX;
        int nY = stSMCOR.Common.MapY;

        int nAction    = stSMCOR.Common.Action;
        int nDirection = stSMCOR.Common.Direction;
        int nSpeed     = stSMCOR.Common.Speed;

        auto pRecord = m_CreatureRecord.find(stSMCOR.Common.UID);
        if(pRecord == m_CreatureRecord.end()){
            switch(stSMCOR.Type){
                case CREATURE_MONSTER:
                    {
                        auto pMonster = new Monster(stSMCOR.Common.UID, stSMCOR.Monster.MonsterID, this, nX, nY, nAction, nDirection, nSpeed);
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
                pRecord->second->OnReportAction(nAction, nDirection, nSpeed, nX, nY);
            }
        }
    }
}
