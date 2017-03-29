/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 03/28/2017 18:06:35
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

#include "actorpod.hpp"
#include "monoserver.hpp"
#include "charobject.hpp"
#include "messagepack.hpp"
#include "protocoldef.hpp"
#include "eventtaskhub.hpp"

CharObject::CharObject(ServiceCore *pServiceCore,
        ServerMap                  *pServerMap,
        int                         nMapX,
        int                         nMapY,
        int                         nDirection,
        uint8_t                     nLifeState,
        uint8_t                     nActionState)
    : ActiveObject()
    , m_ServiceCore(pServiceCore)
    , m_Map(pServerMap)
    , m_CurrX(nMapX)
    , m_CurrY(nMapY)
    , m_Direction(nDirection)
    , m_Ability()
    , m_WAbility()
    , m_AddAbility()
{
    ResetState(STATE_LIFECYCLE, nLifeState);
    ResetState(STATE_ACTION,    nActionState);
}

bool CharObject::NextLocation(int *pX, int *pY, int nDistance)
{
    if(m_Direction >= 0 && m_Direction < 8){
        static const int nDX[] = { 0, +1, +1, +1,  0, -1, -1, -1};
        static const int nDY[] = {-1,  0,  0, +1, +1,  0,  0, -1};

        if(pX){ *pX = m_CurrX + (nDX[m_Direction] * nDistance); }
        if(pY){ *pY = m_CurrY + (nDY[m_Direction] * nDistance); }

        return true;
    }

    return false;
}

uint8_t CharObject::Direction(int nX, int nY)
{
    int nDX = nX - m_CurrX;
    int nDY = nY - m_CurrY;

    uint8_t nDirection = 0;
    if(nDX == 0){
        if(nDY > 0){
            nDirection = 4;
        }else{
            nDirection = 0;
        }
    }else{
        double dATan = std::atan(1.0 * nDY / nDX);
        if(nDX > 0){
            nDirection = (uint8_t)(std::lround(2.0 + dATan * 4.0 / 3.1416) % 8);
        }else{
            nDirection = (uint8_t)(std::lround(6.0 + dATan * 4.0 / 3.1416) % 8);
        }
    }
    return nDirection;
}

void CharObject::DispatchAction()
{
    if(ActorPodValid() && m_Map->ActorPodValid()){
        AMActionState stAMAS;
        std::memset(&stAMAS, 0, sizeof(stAMAS));

        stAMAS.UID   = UID();
        stAMAS.X     = X();
        stAMAS.Y     = Y();
        stAMAS.MapID = m_Map->ID();

        stAMAS.Speed     = Speed();
        stAMAS.Action    = (uint8_t)(Action());
        stAMAS.Direction = (uint8_t)(Direction());

        m_ActorPod->Forward({MPK_ACTIONSTATE, stAMAS}, m_Map->GetAddress());
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "can't dispatch action state");
    g_MonoServer->Restart();
}

int CharObject::Action()
{
    switch(State(STATE_ACTION)){
        case STATE_STAND:
            {
                return ACTION_STAND;
            }
        case STATE_WALK:
            {
                return ACTION_WALK;
            }
        case STATE_ATTACK:
            {
                return ACTION_ATTACK;
            }
        case STATE_DIE:
            {
                return ACTION_DIE;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unknown action state");
                g_MonoServer->Restart();

                // to make the compiler happy
                return ACTION_UNKNOWN;
            }
    }
}
