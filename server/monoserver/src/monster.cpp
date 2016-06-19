/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 06/19/2016 11:56:20
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

Monster::Monster(uint32_t nMonsterID)
    : CharObject()
    , m_FreezeWalk(false)
    , m_MonsterID(nMonsterID)
    , m_ActorRecordV()
{
    m_RMAddress = Theron::Address::Null();
    ResetType(OBJECT_ANIMAL,  1);
    ResetType(OBJECT_MONSTER, 1);
}

Monster::~Monster()
{}

bool Monster::Update()
{
    if(!m_MapID){ return false; }

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
                // 1. if we are standing, it lasts at least for 0.5 s
                if(StateTime(STATE_ACTION) < 500.0){ return true; }

                // 2. already 0.5s, then we have probability of 0.1 to turn state, this probability
                // is small since the Update() call is so frequent
                if(std::rand() % 100 > 30){ return true; }

                // 3. we decide to walk, most likely we just walk ``ahead" or do a little turn
                int nOldDir = m_Direction;
                m_Direction = ((m_Direction + stRandomPick.Pick()) % 8);

                // 4. ooop nothing changes
                if(nOldDir == m_Direction){ return true; }

                // 5. we do have a direction update
                ResetState(STATE_ACTION, STATE_WALK);
                ResetStateTime(STATE_ACTION);

                DispatchAction();
                UpdateLocation();

                return true;
            }
        case STATE_WALK:
            {
                // 1. to indicate if we keep walking
                bool bWalk   = true;
                bool bUpdate = false;

                // 2. only happen after 2s
                if(StateTime(STATE_ACTION) >= 2000){
                    if((std::rand() % 100) < 50){
                        // ok we decide to continue to move
                        ResetStateTime(STATE_ACTION);
                    }else{
                        // we decide to update motion state
                        if((std::rand() % 100) < 50){
                            // ok we decide to change to stand
                            ResetState(STATE_ACTION, STATE_STAND);
                            ResetStateTime(STATE_ACTION);

                            // mark that we stopped
                            bWalk   = false;
                            bUpdate = true;
                        }else{
                            // we change direction to move
                            int nOldDir = m_Direction;
                            m_Direction = ((m_Direction + stRandomPick.Pick()) % 8);

                            if(nOldDir == m_Direction){
                                bUpdate = false;
                            }else{
                                ResetStateTime(STATE_ACTION);
                                bUpdate = true;
                            }
                        }
                    }
                }

                if(bWalk  ){ UpdateLocation(); }
                if(bUpdate){ DispatchAction(); }

                return true;
            }
        default:
            {
                return true;
            }
    }
    return false;
}

bool Monster::UpdateLocation()
{
    int nX, nY;
    NextLocation(&nX, &nY, Speed());
    ReportMove(nX, nY);

    return true;
}

void Monster::SpaceMove(const char *szAddr, int nX, int nY)
{
    AMTrySpaceMove stAMTSM;
    stAMTSM.UID     = m_UID;
    stAMTSM.AddTime = m_AddTime;

    stAMTSM.X = nX;
    stAMTSM.Y = nY;
    stAMTSM.R = m_R;

    stAMTSM.CurrX = X();
    stAMTSM.CurrY = Y();
    stAMTSM.MapID = m_MapID;

    auto fnOP = [this, nX, nY](const MessagePack &rstMPK, const Theron::Address &rstAddr){
        switch(rstMPK.Type()){
            case MPK_OK:
                {
                    auto fnROP = [this, rstAddr, nMPKID = rstMPK.ID(), nX, nY](const MessagePack &, const Theron::Address &){
                        // commit the move into new RM
                        m_ActorPod->Forward(MPK_OK, rstAddr, nMPKID);
                        m_RMAddress = rstAddr;
                        m_CurrX = nX;
                        m_CurrY = nY;

                        // TODO
                        // set the proper map id if needed
                        m_FreezeWalk = false;

                        // 2. dispatch motion
                        DispatchAction();
                    };

                    // leave previous RM
                    AMLeave stAML;
                    stAML.UID = m_UID;
                    stAML.AddTime = m_AddTime;
                    m_ActorPod->Forward({MPK_LEAVE, stAML}, m_RMAddress, fnROP);

                    break;
                }
            case MPK_ERROR:
            case MPK_PENDING:
                {
                    // space move failed
                    m_FreezeWalk = false;

                    // we turn back to go or just stand??
                    m_Direction = ((m_Direction + 4) % 8);
                    ResetStateTime(STATE_ACTION);
                    DispatchAction();
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

bool Monster::ReportMove(int nX, int nY)
{
    AMTryMove stAMTM;
    stAMTM.UID = m_UID;
    stAMTM.AddTime = m_AddTime;

    stAMTM.X = nX;
    stAMTM.Y = nY;
    stAMTM.R = m_R;

    stAMTM.MapID = m_MapID;

    auto fnOP = [this, nX, nY](const MessagePack &rstMPK, const Theron::Address &rstAddr){
        switch(rstMPK.Type()){
            case MPK_OK:
                {
                    m_CurrX = nX;
                    m_CurrY = nY;
                    // 1. commit move
                    m_ActorPod->Forward(MPK_OK, rstAddr, rstMPK.ID());
                    m_FreezeWalk = false;

                    // 2. dispatch motion
                    DispatchAction();
                    break;
                }
            case MPK_ADDRESS:
                {
                    // don't have to respond
                    // make communication to the new RM
                    SpaceMove((const char *)rstMPK.Data(), nX, nY);
                    break;
                }
            case MPK_ERROR:
            case MPK_PENDING:
                {
                    // move failed
                    m_FreezeWalk = false;

                    // we trun back to go
                    m_Direction = ((m_Direction + 4) % 8);
                    ResetStateTime(STATE_ACTION);
                    DispatchAction();
                    break;
                }
            default:
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported response type for MPK_TRYMOVE: %s", rstMPK.Name());
                    g_MonoServer->Restart();
                }
        }
    };

    m_FreezeWalk = true;
    return m_ActorPod->Forward({MPK_TRYMOVE, stAMTM}, m_RMAddress, fnOP);
}

uint8_t Monster::Type(uint8_t nType)
{
    return m_TypeV[nType];
}

bool Monster::ResetType(uint8_t nType, uint8_t nThisType)
{
    m_TypeV[nType] = nThisType;
    return true;
}

uint8_t Monster::State(uint8_t nState)
{
    return m_StateV[nState];
}

bool Monster::ResetState(uint8_t nState, uint8_t nThisState)
{
    m_StateV[nState] = nThisState;
    return true;
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
        extern MemoryPN *g_MemoryPN;
        auto pMem = (SMCORecord *)g_MemoryPN->Get(sizeof(SMCORecord));

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
        pMem->Common.R         = R();
        pMem->Common.Action    = (uint32_t)Action();
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
