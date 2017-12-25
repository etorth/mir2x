/*
 * =====================================================================================
 *
 *       Filename: processrunnet.cpp
 *        Created: 08/31/2015 03:43:46
 *  Last Modified: 12/23/2017 02:01:34
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

#include "log.hpp"
#include "game.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "sysconst.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"

// we get all needed initialization info for init the process run
void ProcessRun::Net_LOGINOK(const uint8_t *pBuf, size_t nLen)
{
    if(pBuf && nLen && (nLen == sizeof(SMLoginOK))){
        SMLoginOK stSMLOK;
        std::memcpy(&stSMLOK, pBuf, nLen);

        uint32_t nUID     = stSMLOK.UID;
        uint32_t nDBID    = stSMLOK.DBID;
        bool     bGender  = stSMLOK.Male;
        uint32_t nMapID   = stSMLOK.MapID;
        uint32_t nDressID = 0;

        int nX = stSMLOK.X;
        int nY = stSMLOK.Y;
        int nDirection = stSMLOK.Direction;

        LoadMap(nMapID);
        m_MyHero = new MyHero(nUID, nDBID, bGender, nDressID, this, ActionStand(nX, nY, nDirection));

        CenterMyHero();
        m_CreatureRecord[m_MyHero->UID()] = m_MyHero;
    }
}

void ProcessRun::Net_ACTION(const uint8_t *pBuf, size_t)
{
    SMAction stSMA;
    std::memcpy(&stSMA, pBuf, sizeof(stSMA));

    ActionNode stAction
    {
        stSMA.Action,
        stSMA.Speed,
        stSMA.Direction,
        stSMA.X,
        stSMA.Y,
        stSMA.AimX,
        stSMA.AimY,
        stSMA.AimUID,
        stSMA.ActionParam,
    };

    if(stSMA.MapID == MapID()){
        if(auto pCreature = RetrieveUID(stSMA.UID)){
            pCreature->ParseAction(stAction);
            switch(stAction.Action){
                case ACTION_SPACEMOVE2:
                    {
                        if(stSMA.UID == m_MyHero->UID()){
                            CenterMyHero();
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }else{
            // can't find it
            // we have to create a new actor but need more information
            CMQueryCORecord stCMQCOR;
            stCMQCOR.UID   = stSMA.UID;
            stCMQCOR.MapID = stSMA.MapID;
            stCMQCOR.X     = stSMA.X;
            stCMQCOR.Y     = stSMA.Y;

            extern Game *g_Game;
            g_Game->Send(CM_QUERYCORECORD, stCMQCOR);
        }
    }else{
        if(m_MyHero && m_MyHero->UID() == stSMA.UID){
            LoadMap(stSMA.MapID);

            auto nUID       = m_MyHero->UID();
            auto nDBID      = m_MyHero->DBID();
            auto bGender    = m_MyHero->Gender();
            auto nDress     = m_MyHero->Dress();
            auto nDirection = m_MyHero->CurrMotion().Direction;

            auto nX = stSMA.X;
            auto nY = stSMA.Y;

            ClearCreature();
            m_MyHero = new MyHero(nUID, nDBID, bGender, nDress, this, ActionStand(nX, nY, nDirection));
            m_CreatureRecord[m_MyHero->UID()] = m_MyHero;

            CenterMyHero();
            m_MyHero->ParseAction(stAction);
        }
    }
}

void ProcessRun::Net_CORECORD(const uint8_t *pBuf, size_t)
{
    SMCORecord stSMCOR;
    std::memcpy(&stSMCOR, pBuf, sizeof(stSMCOR));

    if(stSMCOR.Common.MapID == MapID()){
        ActionNode stAction
        {
            stSMCOR.Common.Action,
            stSMCOR.Common.Speed,
            stSMCOR.Common.Direction,
            stSMCOR.Common.X,
            stSMCOR.Common.Y,
            stSMCOR.Common.EndX,
            stSMCOR.Common.EndY,
            0,
            stSMCOR.Common.ActionParam,
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
                        auto pHero = new Hero(stSMCOR.Common.UID, stSMCOR.Player.DBID, true, 0, this, stAction);
                        m_CreatureRecord[stSMCOR.Common.UID] = pHero;
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }else{
            if(pRecord->second){
                pRecord->second->ParseAction(stAction);
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

void ProcessRun::Net_EXP(const uint8_t *pBuf, size_t)
{
    SMExp stSME;
    std::memcpy(&stSME, pBuf, sizeof(stSME));

    if(stSME.Exp){
        AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"你获得了经验值%d", (int)(stSME.Exp));
    }
}

void ProcessRun::Net_SHOWDROPITEM(const uint8_t *pBuf, size_t)
{
    SMShowDropItem stSMSDI;
    std::memcpy(&stSMSDI, pBuf, sizeof(stSMSDI));

    RemoveGroundItem(0, stSMSDI.X, stSMSDI.Y);
    for(size_t nIndex = 0; nIndex < std::extent<decltype(stSMSDI.IDList)>::value; ++nIndex){
        if(stSMSDI.IDList[nIndex]){
            AddGroundItem(stSMSDI.IDList[nIndex], stSMSDI.X, stSMSDI.Y);
        }else{
            break;
        }
    }
}

void ProcessRun::Net_FIREMAGIC(const uint8_t *pBuf, size_t)
{
    SMFireMagic stSMFM;
    std::memcpy(&stSMFM, pBuf, sizeof(stSMFM));

    if(auto &rstMR = DBCOM_MAGICRECORD(stSMFM.Magic)){
        AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"使用魔法: %s", rstMR.Name);

        switch(stSMFM.Magic){
            case DBCOM_MAGICID(u8"魔法盾"):
                {
                    if(auto stEntry = rstMR.GetGfxEntry(u8"开始")){
                        if(auto pCreature = RetrieveUID(stSMFM.UID)){
                            pCreature->AddAttachMagic(stSMFM.Magic, 0, stEntry.Stage);
                        }
                        return;
                    }
                }
        }

        const GfxEntry *pEntry = nullptr;
        if(stSMFM.UID != m_MyHero->UID()){
            if(!(pEntry && *pEntry)){ pEntry = &(rstMR.GetGfxEntry(u8"启动")); }
            if(!(pEntry && *pEntry)){ pEntry = &(rstMR.GetGfxEntry(u8"开始")); }
            if(!(pEntry && *pEntry)){ pEntry = &(rstMR.GetGfxEntry(u8"运行")); }
            if(!(pEntry && *pEntry)){ pEntry = &(rstMR.GetGfxEntry(u8"结束")); }
        }else{
            if(!(pEntry && *pEntry)){ pEntry = &(rstMR.GetGfxEntry(u8"开始")); }
            if(!(pEntry && *pEntry)){ pEntry = &(rstMR.GetGfxEntry(u8"运行")); }
            if(!(pEntry && *pEntry)){ pEntry = &(rstMR.GetGfxEntry(u8"结束")); }
        }

        if(pEntry && *pEntry){
            switch(pEntry->Type){
                case EGT_BOUND:
                    {
                        if(auto pCreature = RetrieveUID(stSMFM.AimUID)){
                            pCreature->AddAttachMagic(stSMFM.Magic, 0, pEntry->Stage);
                        }
                        break;
                    }
                case EGT_FIXED:
                    {
                        auto pMagic = new IndepMagic
                        {
                            stSMFM.UID,
                            stSMFM.Magic,
                            stSMFM.MagicParam,
                            pEntry->Stage,
                            stSMFM.Direction,
                            stSMFM.X,
                            stSMFM.Y,
                            stSMFM.AimX,
                            stSMFM.AimY,
                            stSMFM.AimUID
                        };

                        m_IndepMagicList.emplace_back(pMagic);
                        break;
                    }
                case EGT_SHOOT:
                    {
                        break;
                    }
                case EGT_FOLLOW:
                    {
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }
}

void ProcessRun::Net_OFFLINE(const uint8_t *pBuf, size_t)
{
    SMOffline stSMO;
    std::memcpy(&stSMO, pBuf, sizeof(stSMO));

    if(stSMO.MapID == MapID()){
        if(auto pCreature = RetrieveUID(stSMO.UID)){
            pCreature->AddAttachMagic(DBCOM_MAGICID(u8"瞬息移动"), 0, EGS_INIT);
        }
    }
}

void ProcessRun::Net_PICKUPOK(const uint8_t *pBuf, size_t)
{
    SMPickUpOK stSMPUOK;
    std::memcpy(&stSMPUOK, pBuf, sizeof(stSMPUOK));

    m_MyHero->GetInvPack().Add(stSMPUOK.ItemID);
    RemoveGroundItem(stSMPUOK.ItemID, stSMPUOK.X, stSMPUOK.Y);
    AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"捡起%s于坐标(%d, %d)", DBCOM_ITEMRECORD(stSMPUOK.ItemID).Name, (int)(stSMPUOK.X), (int)(stSMPUOK.Y));
}
