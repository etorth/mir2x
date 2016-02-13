/*
 * =====================================================================================
 *
 *       Filename: actor.cpp
 *        Created: 9/3/2015 5:55:13 PM
 *  Last Modified: 09/09/2015 6:51:42 PM
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

#include "actor.hpp"
#include "sceneserver.hpp"
#include "servermessagedef.hpp"

std::vector<std::tuple<int, int, DirectiveRectCover>> Actor::m_SIDStateDRCTupleV;

Actor::Actor(int nSID, int nUID, int nGenTime)
    : m_SID(nSID)
    , m_UID(nUID)
    , m_GenTime(nGenTime)
    , m_State(0)
    , m_Direction(0)
    , m_X(0)
    , m_Y(0)
    , m_HP(0)
    , m_DRCIndex(-1)
    , m_Map(nullptr)
{
    extern SceneServer *g_SceneServer;
    m_LastSyrcTime = g_SceneServer->GetTimeMS();
}

Actor::~Actor()
{}

int Actor::State() const
{
    return m_State;
}

int Actor::Direction() const
{
    return m_Direction;
}

int Actor::SID() const
{
    return m_SID;
}

int Actor::UID() const
{
    return m_UID;
}

int Actor::GenTime() const
{
    return m_GenTime;
}

int Actor::HP() const
{
    return m_HP;
}

int Actor::X() const
{
    return m_X;
}

int Actor::Y() const
{
    return m_Y;
}

void Actor::SetMap(int nX, int nY, NewMir2Map *pMap)
{
    m_X   = nX;
    m_Y   = nY;
    m_Map = pMap;
}

bool Actor::Collide(const Actor *pActor)
{
    SetDRCIndex();
    DirectiveRectCover stDRC1(
            Direction(),
            m_X,
            m_Y,
            DRCover().W(),
            DRCover().H());

    DirectiveRectCover stDRC2(
            pActor->Direction(),
            pActor->X(),
            pActor->Y(),
            pActor->DRCover().W(),
            pActor->DRCover().H());

    return stDRC1.Overlap(stDRC2);
}

const DirectiveRectCover &Actor::DRCover() const
{
    return std::get<2>(m_SIDStateDRCTupleV[m_DRCIndex]);
}

bool Actor::RandomStart()
{
    if(!GlobalCoverInfoValid(m_SID)){
        return false;
    }

    switch(m_SID){
        case 1000:
            {
                m_State     = STATE_STAND;
                m_Direction = 5;
                break;
            }
        default:
            {
                m_State     = STATE_STAND;
                m_Direction = 5;
                break;
            }
    }

    return SetDRCIndex();
}

void Actor::BroadcastBaseInfo()
{
    ServerMessageActorBaseInfo stTmpSM;
    stTmpSM.nSID       = m_SID;
    stTmpSM.nUID       = m_UID;
    stTmpSM.nGenTime   = m_GenTime;
    stTmpSM.nX         = m_X;
    stTmpSM.nY         = m_Y;
    stTmpSM.nState     = m_State;
    stTmpSM.nDirection = m_Direction;

    Message stTmpMSG;
    stTmpMSG.Set(SERVERMT_ACTORBASEINFO, stTmpSM);

    extern SceneServer *g_SceneServer;
    g_SceneServer->Broadcast(stTmpMSG);
}

void Actor::SetDirection(int nDirection)
{
    m_Direction = nDirection;
}

bool Actor::SetDRCIndex()
{
    if(m_DRCIndex < 0){
        m_DRCIndex = GlobalCoverInfoIndex();
    }
    return false;
}

int Actor::GlobalCoverInfoIndex(int nSID, int nState, int nDirection)
{
    switch(((uint32_t)nSID & 0XFFFFFC00) >> 10){
        case 1:
            { // monster
                nSID = nSID;
                break;
            }
        case 3:
            { // human
                nSID = (3 << 10);
                break;
            }
        default:
            {
                nSID = nSID;
                break;
            }
    }

    for(int nCnt = 0; nCnt < m_SIDStateDRCTupleV.size(); ++nCnt){
        if(true
                && std::get<0>(m_SIDStateDRCTupleV[nCnt]) == nSID
                && std::get<1>(m_SIDStateDRCTupleV[nCnt]) == m_State
                && std::get<2>(m_SIDStateDRCTupleV[nCnt]).Direction() == m_Direction
          ){
            return nCnt;
        }
    }

    return -1;
}

bool Actor::GlobalCoverInfoValid(int nSID)
{
    switch(((uint32_t)nSID & 0XFFFFFC00) >> 10){
        case 1:
            { // monster
                nSID = nSID;
                break;
            }
        case 3:
            { // human
                nSID = (3 << 10);
                break;
            }
        default:
            {
                nSID = nSID;
                break;
            }
    }

    // TODO currently it's a simple implementation
    for(auto &stDRCP: m_SIDStateDRCTupleV){
        if(std::get<0>(stDRCP) == nSID){
            return true;
        }
    }
    return false;
}

bool Actor::SetGlobalCoverInfo(int nSID, int nState, int nDir, int nW, int nH)
{
    switch(((uint32_t)nSID & 0XFFFFFC00) >> 10){
        case 1:
            {
                nSID = nSID;
                break;
            }
        case 3:
            {
                nSID = (3 << 10);
                break;
            }
        default:
            {
                break;
            }

    }
    m_SIDStateDRCTupleV.emplace_back(
            nSID, nState, DirectiveRectCover(nDir, 0.0, 0.0, (double)nW, (double)nH));
    return true;
}
