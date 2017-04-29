/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/28/2017 23:06:04
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
{
    ResetType(TYPE_CHAR, TYPE_MONSTER);
    ResetType(TYPE_MONSTER, TYPE_MONSTER);

    ResetType(TYPE_CREATURE, TYPE_ANIMAL);
    ResetType(TYPE_ANIMAL, TYPE_ANIMAL);
}

bool Monster::Update()
{
    if(CanMove()){
        // always try to move if possible
        {
            int nNextX = 0;
            int nNextY = 0;
            if(NextLocation(&nNextX, &nNextY, 1)){
                if(m_Map->GroundValid(nNextX, nNextY)){
                    return RequestMove(nNextX, nNextY, [](){}, [](){});
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

void Monster::Operate(const MessagePack &rstMPK, const Theron::Address &rstAddress)
{
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK, rstAddress);
                break;
            }
        case MPK_MAPSWITCH:
            {
                On_MPK_MAPSWITCH(rstMPK, rstAddress);
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
        SMCORecord stSMCOR;
        // TODO: don't use OBJECT_MONSTER, we need translation
        //       rule of communication, the sender is responsible to translate

        // 1. set type
        stSMCOR.Type = CREATURE_MONSTER;

        // 2. set common info
        stSMCOR.Common.UID   = UID();
        stSMCOR.Common.MapID = MapID();

        stSMCOR.Common.Action      = ACTION_STAND;
        stSMCOR.Common.ActionParam = 0;
        stSMCOR.Common.Speed       = 0;
        stSMCOR.Common.Direction   = Direction();

        stSMCOR.Common.X    = X();
        stSMCOR.Common.Y    = Y();
        stSMCOR.Common.EndX = X();
        stSMCOR.Common.EndY = Y();

        // 3. set specified info
        stSMCOR.Monster.MonsterID = m_MonsterID;

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(nSessionID, SM_CORECORD, stSMCOR);
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid session id");
        g_MonoServer->Restart();
    }
}

int Monster::Range(uint8_t)
{
    return 20;
}

int Monster::Speed()
{
    return 1;
}
