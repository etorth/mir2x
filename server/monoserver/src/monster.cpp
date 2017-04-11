/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/11/2017 12:28:08
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
        uint8_t             nLifeState)
    : CharObject(pServiceCore, pServerMap, nMapX, nMapY, nDirection, nLifeState)
    , m_MonsterID(nMonsterID)
    , m_FreezeWalk(false)
{
    ResetType(TYPE_CHAR, TYPE_MONSTER);
    ResetType(TYPE_CREATURE, TYPE_ANIMAL);
}

bool Monster::Update()
{
    if(!m_FreezeWalk){
        // always try to move if possible
        {
            int nNextX = 0;
            int nNextY = 0;
            if(NextLocation(&nNextX, &nNextY, 1)){
                if(m_Map->GroundValid(nNextX, nNextY)){
                    return RequestMove(nNextX, nNextY);
                }
            }
        }

        // if current direction leads to a *impossible* place
        // then randomly take a new direction to try
        {
            static const int nDirV[] = {
                DIR_UP,
                DIR_UPRIGHT,
                DIR_RIGHT,
                DIR_DOWNRIGHT,
                DIR_DOWN,
                DIR_DOWNLEFT,
                DIR_LEFT,
                DIR_UPLEFT,
            };

            auto nDirCount = (int)(sizeof(nDirV) / sizeof(nDirV[0]));
            auto nDirStart = (int)(std::rand() % nDirCount);

            for(int nIndex = 0; nIndex < nDirCount; ++nIndex){
                auto nDirection = nDirV[(nDirStart + nIndex) % nDirCount];
                if(nDirection != m_Direction){
                    int nNextX = 0;
                    int nNextY = 0;
                    if(NextLocation(&nNextX, &nNextY, nDirection, 1)){
                        if(m_Map->GroundValid(nNextX, nNextY)){
                            m_Direction = nDirection;
                            DispatchAction({ACTION_STAND, 0, m_Direction, X(), Y()});
                            return true;
                        }
                    }
                }
            }

            // need future work here
            // ooops, we are at a place can't move
            {
                return false;
            }
        }
    }
    return true;
}

void Monster::RequestSpaceMove(const char *szAddr, int nX, int nY)
{
    AMTrySpaceMove stAMTSM;
    std::memset(&stAMTSM, 0, sizeof(stAMTSM));

    stAMTSM.UID = UID();
    stAMTSM.X   = nY;
    stAMTSM.Y   = nY;

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
    if(m_FreezeWalk){
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
        {
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_INFO, "shouldn't request move when walk freezed");
        }
#endif
        return false;
    }

    AMTryMove stAMTM;
    std::memset(&stAMTM, 0, sizeof(stAMTM));

    stAMTM.This    = this;
    stAMTM.UID     = UID();
    stAMTM.X       = nX;
    stAMTM.Y       = nY;
    stAMTM.MapID   = m_Map->ID();
    stAMTM.CurrX   = X();
    stAMTM.CurrY   = Y();

    auto fnOP = [this, nX, nY](const MessagePack &rstMPK, const Theron::Address &rstAddr){
        switch(rstMPK.Type()){
            case MPK_OK:
                {
                    static const int nDirV[][3] = {
                        {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
                        {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
                        {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

                    int nDX = nX - m_CurrX + 1;
                    int nDY = nY - m_CurrY + 1;

                    ActionNode stAction;
                    stAction.Action      = ACTION_MOVE;
                    stAction.ActionParam = 0;
                    stAction.Speed       = 1;
                    stAction.Direction   = nDirV[nDY][nDX];
                    stAction.X           = m_CurrX;
                    stAction.Y           = m_CurrY;
                    stAction.EndX        = nX;
                    stAction.EndY        = nY;

                    m_CurrX = nX;
                    m_CurrY = nY;
                    m_ActorPod->Forward(MPK_OK, rstAddr, rstMPK.ID());

                    DispatchAction(stAction);
                    m_FreezeWalk = false;
                    break;
                }
            case MPK_ERROR:
                {
                    m_Direction = GetBack();
                    DispatchAction({ACTION_STAND, 0, m_Direction, X(), Y()});
                    m_FreezeWalk = false;
                    break;
                }
            default:
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_FATAL, "unsupported response type for MPK_TRYMOVE: %s", rstMPK.Name());
                    g_MonoServer->Restart();
                    break;
                }
        }
    };

    m_FreezeWalk = true;
    return m_ActorPod->Forward({MPK_TRYMOVE, stAMTM}, m_Map->GetAddress(), fnOP);
}

void Monster::Operate(const MessagePack &rstMPK, const Theron::Address &rstAddress)
{
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK, rstAddress);
                break;
            }
        case MPK_PULLCOINFO:
            {
                On_MPK_PULLCOINFO(rstMPK, rstAddress);
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
        pMem->Common.UID   = UID();
        pMem->Common.MapID = MapID();

        pMem->Common.Action      = ACTION_STAND;
        pMem->Common.ActionParam = 0;
        pMem->Common.Speed       = 0;
        pMem->Common.Direction   = Direction();

        pMem->Common.X    = X();
        pMem->Common.Y    = Y();
        pMem->Common.EndX = X();
        pMem->Common.EndY = Y();

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

int Monster::Range(uint8_t)
{
    return 20;
}

int Monster::Speed()
{
    return 1;
}
