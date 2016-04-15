/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/14/2016 17:33:13
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

bool Monster::Operate()
{
    // don't try to suiside in yourself here
    if(!Active()){ return false; }

    std::printf("moster (%d, %d) is now at (%d, %d)\n", UID(), AddTime(), X(), Y());

    extern MonoServer *g_MonoServer;
    // no target, then try to find one from the view range
    if(m_Target == EMPTYOBJECTRECORD && !m_VisibleObjectList.empty()){
        // have to use while since list may change
        auto pRecord = m_VisibleObjectList.begin();
        while(pRecord != m_VisibleObjectList.end()){
            if(auto pGuard = g_MonoServer->CheckOut<CharObject>(
                        std::get<0>(*pRecord), std::get<1>(*pRecord))){
                if(true
                        && pGuard->Active()
                        && pGuard->MapID() == m_Map->ID()
                        && LDistance(pGuard->X(), pGuard->Y(), m_CurrX, m_CurrY) <= Range(RANGE_VIEW)){
                    // valid target, set it
                    m_Target = *pRecord;
                    break;
                }

            }
            // not valid or not proper, erase it
            // valid means still in the object hub
            // proper means it stays in the view range
            pRecord = m_VisibleObjectList.erase(pRecord);
        }
    }

    // still need to validate the target
    if(m_Target != EMPTYOBJECTRECORD){
        if(auto pGuard = g_MonoServer->CheckOut<CharObject>(
                    std::get<0>(m_Target), std::get<1>(m_Target))){
            if(pGuard->Active() && pGuard->MapID() == m_Map->ID()){
                int nR = LDistance(pGuard->X(), pGuard->Y(), m_CurrX, m_CurrY);
                if(nR <= Range(RANGE_ATTACK)){
                    return Attack(pGuard.Get());
                }

                if(nR <= Range(RANGE_TRACETARGET)){
                    return Follow(pGuard.Get(), false);
                }
            }
        }
    }

    // now we don't have a target
    m_Target = EMPTYOBJECTRECORD;

    // try master
    if(auto pGuard = g_MonoServer->CheckOut<CharObject>(
                std::get<0>(m_Master), std::get<1>(m_Master))){
        if(pGuard && pGuard->Active()){
            return Follow(pGuard.Get(), true);
        }
    }

    // no master
    m_Master = EMPTYOBJECTRECORD;

    if(std::rand() % 5 == 0){
        return RandomWalk();
    }

    return false;
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
    if(!State(STATE_INCARNATED)){ return false; }

    int nX, nY;
    NextLocation(&nX, &nY, Speed());

    return m_Map->ObjectMove(nX, nY, this);
}

// get a cached object list, by reference only, test each object in the list
// before using it, see TODO & TBD in charobject.hpp for this function
void Monster::SearchViewRange()
{
    int nRange = Range(RANGE_VIEW);

    int nStartX = m_CurrX - nRange;
    int nStopX  = m_CurrX + nRange;
    int nStartY = m_CurrY - nRange;
    int nStopY  = m_CurrY + nRange;

    int nGridX0 = nStartX / 48;
    int nGridY0 = nStartY / 32;
    int nGridX1 = nStopX  / 48;
    int nGridY1 = nStopY  / 32;

    // 1. clear the cached object
    m_VisibleObjectList.clear();

    // 2. update grid one by one
    for(int nY = nGridY0; nY <= nGridY1; nY++){
        for(int nX = nGridX0; nX <= nGridX1; nX++){
            if(!m_Map->ValidC(nX, nY)){ continue; }

            auto fnQueryObject = [this](uint8_t nType, uint32_t nID, uint32_t nAddTime){
                if(nType != OBJECT_PLAYER){ return; }
                m_VisibleObjectList.emplace_back(nID, nAddTime);
            };

            m_Map->QueryObject(nX, nY, fnQueryObject);
        }
    }
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
