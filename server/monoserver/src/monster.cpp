/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 05/04/2016 00:26:58
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

Monster::Monster(uint32_t nMonsterInex, uint32_t nUID, uint32_t nAddTime)
    : CharObject(nUID, nAddTime)
    , m_MonitorAddress(Theron::Address::Null())
    , m_MonsterIndex(nMonsterInex)
{
    SetState(STATE_INCARNATED, true);
    SetState(STATE_CANMOVE   , true);
    SetState(STATE_WAITMOVE  , true);
}

Monster::~Monster()
{}

bool Monster::Update()
{
    // don't try to suiside in yourself here
    // if(!Active()){ return false; }

    std::printf("moster (%d, %d) is now at (%d, %d)\n", UID(), AddTime(), X(), Y());

    // 1. query neighbors
    for(const auto &rstRecord: m_NeighborV){
        m_ActorPod->Forward({MPK_QUERYLOCATION}, rstRecord.PodAddress);
    }

    // // 2. try to find target, using current info of neighbors
    // if(!m_TargetRecord.Valid()){
    //     int nLDis2 = -1;
    //     for(const auto &rstRecord: m_NeighborV){
    //         if(rstRecord.Friend){ continue; }
    //         int nCurrLDis2 = LDistance2(X(), Y(), std::get<4>(rstRecord), std::get<5>(rstRecord));
    //         if(nCurrLDis2 < nLDis2){
    //             m_TargetRecord = rstRecord;
    //             nLDis2 = nCurrLDis2;
    //         }
    //     }
    // }

    // // 3. if now we have target
    // if(m_TargetRecord.Valid()){
    //     // did the attack
    //     if(Attack()){ return true;}
    //
    //     // can't attack, then follow it
    // }

    // if(m_MasterRecord.Valid()){
    // }

    return RandomWalk();
}

bool Monster::Attack(CharObject *pObject)
{
    if(pObject) {
        return true;
    }
    return false;
}

bool Monster::RandomWalk()
{
    // with prob. of 20% to trigger this functioin
    if(std::rand() % 5 > 0) { return false; }

    if(!State(STATE_INCARNATED)){ return false; }
    if(!State(STATE_CANMOVE   )){ return false; }
    if(!State(STATE_WAITMOVE  )){ return false; }

    int nX, nY;
    NextLocation(&nX, &nY, Speed());
    ReportMove(nX, nY);

    return true;
}

bool Monster::ReportMove(int nX, int nY)
{
    AMTryMove stMPKTM;
    stMPKTM.OldX = X();
    stMPKTM.OldY = Y();
    stMPKTM.X    = nX;
    stMPKTM.Y    = nY;

    return m_ActorPod->Forward({MPK_TRYMOVE, stMPKTM}, m_MonitorAddress);
}

bool Monster::Type(uint8_t nType)
{
    switch(nType){
        case OBJECT_ANIMAL: return true;
        default: return false;
    }
    return false;
}

bool Monster::SetType(uint8_t nType, bool bThisType)
{
    m_TypeV[nType] = 1;
    return bThisType;
}

bool Monster::Follow(CharObject *, bool)
{
    return true;
}

bool Monster::State(uint8_t nState)
{
    return m_StateV[nState];
}

bool Monster::SetState(uint8_t nState, bool bThisState)
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

        case MPK_MOVEOK:
            {
                On_MPK_MOVEOK(rstMPK, rstAddress);
                break;
            }

        case MPK_LOCATIION:
            {
                On_MPK_LOCATIION(rstMPK, rstAddress);
                break;
            }

        case MPK_MASTERPERSONA:
            {
                On_MPK_MASTERPERSONA(rstMPK, rstAddress);
                break;
            }

        default:
            {
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "unsupported message type: %d", rstMPK.Type());
                break;
            }
    }
}

void Monster::SearchViewRange()
{

}
