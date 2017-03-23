/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 03/23/2017 11:28:33
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
    switch(State(STATE_ACTION)){
        case STATE_STAND:
        case STATE_WALK:
            {
                return RandomWalk();
            }
        default:
            {
                // TODO ...
                return true;
            }
    }

    return true;
}

bool Monster::RandomWalk()
{
    // 1. if current the moving is in progress
    if(m_FreezeWalk){ return false; }

    // 2. ok we make a direction generator
    RandomPick<int> stRandomPick;

    stRandomPick.Add(160 + 20, 0);
    stRandomPick.Add( 80 + 20, 1);
    stRandomPick.Add( 80 + 20, 7);
    stRandomPick.Add( 40 + 20, 2);
    stRandomPick.Add( 40 + 20, 6);
    stRandomPick.Add( 20 + 20, 3);
    stRandomPick.Add( 20 + 20, 5);
    stRandomPick.Add( 10 + 20, 4);

    // 3. decide to walk
    switch(State(STATE_ACTION)){
        case STATE_STAND:
            {
                return true;
            }
        case STATE_WALK:
            {
                return true;
            }
        default:
            {
                return false;
            }
    }
}

bool Monster::UpdateLocation()
{
    int nX, nY;
    NextLocation(&nX, &nY, Speed());
    RequestMove(nX, nY);

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

    stAMTM.UID     = m_UID;
    stAMTM.AddTime = m_AddTime;
    stAMTM.X       = nX;
    stAMTM.Y       = nY;
    stAMTM.MapID   = m_Map->ID();

    auto fnOP = [this, nX, nY](const MessagePack &rstMPK, const Theron::Address &rstAddr){
        switch(rstMPK.Type()){
            case MPK_OK:
                {
                    m_CurrX = nX;
                    m_CurrY = nY;
                    m_ActorPod->Forward(MPK_OK, rstAddr, rstMPK.ID());

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
        auto pMem = new SMCORecord();

        // TODO: don't use OBJECT_MONSTER, we need translation
        //       rule of communication, the sender is responsible to translate

        // 1. set type
        pMem->Type = CREATURE_MONSTER;

        // 2. set common info
        pMem->Common.UID       = UID();
        pMem->Common.AddTime   = AddTime();
        pMem->Common.MapID     = MapID();
        pMem->Common.MapX      = X();
        pMem->Common.MapY      = Y();
        pMem->Common.Action    = (uint32_t)Action();
        pMem->Common.Direction = Direction();
        pMem->Common.Speed     = Speed();

        // 3. set specified info
        pMem->Monster.MonsterID = m_MonsterID;

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(nSessionID, SM_CORECORD, (uint8_t *)pMem, sizeof(SMCORecord), [pMem](){ delete pMem; });
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid session id");
    g_MonoServer->Restart();
}
