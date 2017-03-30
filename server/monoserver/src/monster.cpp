/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 03/29/2017 18:37:42
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

// #include <cstdio>

#include "netpod.hpp"
#include "monster.hpp"
#include "actorpod.hpp"
#include "mathfunc.hpp"
#include "memorypn.hpp"
#include "randompick.hpp"
#include "monoserver.hpp"
#include "messagepack.hpp"
#include "protocoldef.hpp"

Monster::Monster(uint32_t   nMonsterID,
        ServiceCore        *pServiceCore,
        ServerMap          *pServerMap,
        int                 nMapX,
        int                 nMapY,
        int                 nDirection,
        uint8_t             nLifeState,
        uint8_t             nActionState)
    : CharObject(pServiceCore, pServerMap, nMapX, nMapY, nDirection, nLifeState, nActionState)
    , m_MonsterID(nMonsterID)
    , m_FreezeWalk(false)
{
    ResetType(TYPE_CHAR, TYPE_MONSTER);
    ResetType(TYPE_CREATURE, TYPE_ANIMAL);
}

bool Monster::Update()
{
    if((std::rand() % 99991) < (99991 / 2)){
        int nNextX = 0;
        int nNextY = 0;
        if(NextLocation(&nNextX, &nNextY, Speed())){
            RequestMove(nNextX, nNextY);
        }
    }else{
        if(std::rand() % 99991 < (99991 / 2)){
            m_Direction = std::rand() % 8;
            DispatchAction();
        }
    }

    return true;
}

void Monster::RequestSpaceMove(const char *szAddr, int nX, int nY)
{
    AMTrySpaceMove stAMTSM;
    std::memset(&stAMTSM, 0, sizeof(stAMTSM));

    stAMTSM.UID     = m_UID;
    stAMTSM.AddTime = m_AddTime;

    stAMTSM.X = nX;
    stAMTSM.Y = nY;

    stAMTSM.CurrX = X();
    stAMTSM.CurrY = Y();
    stAMTSM.MapID = m_Map->ID();

    auto fnOP = [this, nX, nY](const MessagePack &rstMPK, const Theron::Address &rstAddr){
        switch(rstMPK.Type()){
            case MPK_OK:
                {
                    m_ActorPod->Forward(MPK_OK, rstAddr, rstMPK.ID());
                    m_CurrX = nX;
                    m_CurrY = nY;

                    m_FreezeWalk = false;
                    break;
                }
            case MPK_ERROR:
                {
                    m_FreezeWalk = false;
                    break;
                }
            default:
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported response type for MPK_TRYSPACEMOVE: %s", rstMPK.Name());
                    g_MonoServer->Restart();
                }
        }
    };

    m_FreezeWalk = true;
    m_ActorPod->Forward({MPK_TRYSPACEMOVE, stAMTSM}, Theron::Address(szAddr), fnOP);
}

bool Monster::RequestMove(int nX, int nY)
{
    AMTryMove stAMTM;
    std::memset(&stAMTM, 0, sizeof(stAMTM));

    stAMTM.This    = (uintptr_t)(this);
    stAMTM.UID     = m_UID;
    stAMTM.X       = nX;
    stAMTM.Y       = nY;
    stAMTM.MapID   = m_Map->ID();
    stAMTM.CurrX   = m_CurrX;
    stAMTM.CurrY   = m_CurrY;

    auto fnOP = [this, nX, nY](const MessagePack &rstMPK, const Theron::Address &rstAddr){
        switch(rstMPK.Type()){
            case MPK_OK:
                {
                    m_CurrX = nX;
                    m_CurrY = nY;
                    m_ActorPod->Forward(MPK_OK, rstAddr, rstMPK.ID());
                    DispatchAction();

                    m_FreezeWalk = false;
                    break;
                }
            case MPK_ERROR:
                {
                    m_FreezeWalk = false;
                    break;
                }
            default:
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported response type for MPK_TRYMOVE: %s", rstMPK.Name());
                    g_MonoServer->Restart();
                    break;
                }
        }
    };

    m_FreezeWalk = true;
    return m_ActorPod->Forward({MPK_TRYMOVE, stAMTM}, m_Map->GetAddress(), fnOP);
}

uint32_t Monster::NameColor()
{
    return 0XFFFFFFFF;
}

const char *Monster::CharName()
{
    return "hello";
}

int Monster::Range(uint8_t)
{
    return 20;
}

void Monster::Operate(const MessagePack &rstMPK, const Theron::Address &rstAddress)
{
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK, rstAddress);
                break;
            }
        case MPK_HI:
            {
                On_MPK_HI(rstMPK, rstAddress);
                break;
            }
        case MPK_PULLCOINFO:
            {
                On_MPK_PULLCOINFO(rstMPK, rstAddress);
                break;
            }
        case MPK_UPDATECOINFO:
            {
                On_MPK_UPDATECOINFO(rstMPK, rstAddress);
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported message: %s", rstMPK.Name());
                g_MonoServer->Restart();
                break;
            }
    }
}

void Monster::SearchViewRange()
{
}

void Monster::ReportCORecord(uint32_t nSessionID)
{
    if(nSessionID){
        extern MemoryPN *g_MemoryPN;
        auto pMem = (SMCORecord *)g_MemoryPN->Get(sizeof(SMCORecord));

        // TODO: don't use OBJECT_MONSTER, we need translation
        //       rule of communication, the sender is responsible to translate

        // 1. set type
        pMem->Type = CREATURE_MONSTER;

        // 2. set common info
        pMem->Common.UID       = UID();
        pMem->Common.MapID     = MapID();
        pMem->Common.MapX      = X();
        pMem->Common.MapY      = Y();
        pMem->Common.Action    = (uint32_t)(Action());
        pMem->Common.Direction = Direction();
        pMem->Common.Speed     = Speed();

        // 3. set specified info
        pMem->Monster.MonsterID = m_MonsterID;

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(nSessionID, SM_CORECORD, (uint8_t *)pMem, sizeof(SMCORecord), [pMem](){ g_MemoryPN->Free(pMem); });
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid session id");
    g_MonoServer->Restart();
}

int Monster::Speed()
{
    return 1;
}
