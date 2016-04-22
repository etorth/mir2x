/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/21/2016 22:39:59
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

#include "monster.hpp"
#include "mathfunc.hpp"
#include "monoserver.hpp"

Monster::Monster(uint32_t nMonsterInex, uint32_t nUID, uint32_t nAddTime)
    : CharObject(nUID, nAddTime)
    , m_MonsterIndex(nMonsterInex)
    , m_Metronome(nullptr)
{
}

Monster::~Monster()
{}

// TODO & TBD
// 1. will this cause infin-iteration?
// 2. will this cause dead lock?
bool Monster::Friend(CharObject* pCharObject)
{
    if(!pCharObject || pCharObject == this){ return true; }
    if(m_Master != EMPTYOBJECTRECORD){
        extern MonoServer *g_MonoServer;
        if(auto pGuard = g_MonoServer->CheckOut<CharObject>(
                    std::get<0>(m_Master), std::get<1>(m_Master))){
            return pGuard->Friend(pCharObject);
        }
        m_Master = EMPTYOBJECTRECORD;
    }

    // now self is no-master
    if(pCharObject->Type(OBJECT_HUMAN)){
        return false;
    }

    if(pCharObject->Type(OBJECT_ANIMAL)){
        return pCharObject->Friend(this);
    }

    return false;
}

bool Monster::Update()
{
    // don't try to suiside in yourself here
    if(!Active()){ return false; }

    std::printf("moster (%d, %d) is now at (%d, %d)\n", UID(), AddTime(), X(), Y());

    // 1. query neighbors
    for(const auto &rstRecord: m_NeighborV){
        m_ObjectPod->Send(MessagePack(MPK_QUERYLOCATION), rstRecord.Addr);
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

bool ReportMove(int nX, int nY)
{
    MPKTryMove stMPKTM = {
        .OldX = X(),
        .OldY = Y(),
        .X    = nX,
        .Y    = nY,
    };

    return m_ObjectPod->Send(MessagePack(MPK_TRYMOVE, stMPKTM), m_MonitorAddress);
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

void Monster::Operate(const MessagePack &rstMPK, Theron::Address)
{
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                Update();
                break;
            }

        case MPK_MOVEOK:
            {
                MPKMoveOK stMPKMOK;
                std::memcpy(&stMPKMOK, rstMPK.Data(), sizeof(stMPKMOK));

                // 1. do the move
                m_CurrX = stMPKMOK.X;
                m_CurrY = stMPKMOK.Y;
                SetState(STATE_WAITMOVE, false);

                // 2. commit the move to the monitor
                MPKCommitMove stMPKCM = {
                    .X = m_CurrX,
                    .Y = m_CurrY,
                };
                m_ObjectPod->Send(MessagePack(MPK_COMMITMOVE, stMPKCM), m_MonitorAddress);

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
