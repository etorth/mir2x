/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 06/05/2016 22:29:25
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

#include <cstdio>

#include "actorpod.hpp"
#include "monster.hpp"
#include "mathfunc.hpp"
#include "monoserver.hpp"
#include "messagepack.hpp"

Monster::Monster(uint32_t nMonsterID)
    : CharObject()
    , m_FreezeWalk(false)
    , m_MonsterID(nMonsterID)
    , m_ActorRecordV()
{

    m_RMAddress = Theron::Address::Null();

    ResetState(STATE_ACTIVE, false);
    ResetState(STATE_CANMOVE, true);
    ResetState(STATE_WAITMOVE, true);
    ResetState(STATE_INCARNATED, true);
}

Monster::~Monster()
{}

bool Monster::Update()
{
    if(!m_MapID){ return false; }

    std::printf("moster (%d, %d) is now at (%d, %d)\n", UID(), AddTime(), X(), Y());
    return RandomWalk();
}

bool Monster::RandomWalk()
{
    // with prob. of 20% to trigger this functioin
    // if(std::rand() % 5 > 0){ return false; }
    // if(std::rand() % 5 > 0){
        m_Direction = std::rand() % 8;
    // }

    if(m_FreezeWalk){ return false; }

    if(!State(STATE_INCARNATED)){ return false; }
    if(!State(STATE_CANMOVE   )){ return false; }
    if(!State(STATE_WAITMOVE  )){ return false; }

    int nX, nY;
    NextLocation(&nX, &nY, Speed());
    ReportMove(nX, nY);

    return true;
}

void Monster::SpaceMove(const char *szAddr, int nX, int nY)
{
    AMTryMove stAMTSM;
    stAMTSM.UID = m_UID;
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
                        DispatchMotion();
                    };

                    // leave previous RM
                    AMLeave stAML;
                    stAML.UID = m_UID;
                    stAML.AddTime = m_AddTime;
                    m_ActorPod->Forward({MPK_LEAVE, stAML}, m_RMAddress, fnROP);

                    break;
                }
            default:
                {
                    // can only be MPK_ERROR
                    m_FreezeWalk = false;
                    break;
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

    stAMTM.CurrX = X();
    stAMTM.CurrY = Y();

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
                    DispatchMotion();
                    break;
                }
            case MPK_ADDRESS:
                {
                    // don't have to respond
                    // make communication to the new RM
                    SpaceMove((const char *)rstMPK.Data(), nX, nY);
                    break;
                }
            default:
                {
                    // move failed
                    m_FreezeWalk = false;
                    break;
                }
        }
    };

    m_FreezeWalk = true;
    return m_ActorPod->Forward({MPK_TRYMOVE, stAMTM}, m_RMAddress, fnOP);
}

bool Monster::Type(uint8_t nType)
{
    return m_TypeV[nType];
}

bool Monster::ResetType(uint8_t nType, bool bThisType)
{
    m_TypeV[nType] = bThisType;
    return bThisType;
}

bool Monster::State(uint8_t nState)
{
    return m_StateV[nState];
}

bool Monster::ResetState(uint8_t nState, bool bThisState)
{
    m_StateV[nState] = bThisState;
    return bThisState;
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
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported message: type = %d, id = %d, resp = %d", rstMPK.Type(), rstMPK.ID(), rstMPK.Respond());
                break;
            }
    }
}

void Monster::SearchViewRange()
{
}
