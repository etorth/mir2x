/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/09/2017 23:27:55
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
        uint8_t                     nLifeState)
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
}

bool CharObject::NextLocation(int *pX, int *pY, int nDistance)
{
    switch(m_Direction){
        case DIR_UP:
        case DIR_UPRIGHT:
        case DIR_RIGHT:
        case DIR_DOWNRIGHT:
        case DIR_DOWN:
        case DIR_DOWNLEFT:
        case DIR_LEFT:
        case DIR_UPLEFT:
            {
                static const int nDX[] = { 0, +1, +1, +1,  0, -1, -1, -1};
                static const int nDY[] = {-1,  0,  0, +1, +1,  0,  0, -1};

                if(pX){ *pX = m_CurrX + (nDX[m_Direction] * nDistance); }
                if(pY){ *pY = m_CurrY + (nDY[m_Direction] * nDistance); }

                return true;
            }
        default:
            {
                return false;
            }
    }
}

void CharObject::DispatchAction(const ActionNode &rstAction)
{
    if(ActorPodValid() && m_Map->ActorPodValid()){
        AMAction stAMA;
        std::memset(&stAMA, 0, sizeof(stAMA));

        stAMA.UID   = UID();
        stAMA.MapID = m_Map->ID();

        stAMA.Action      = rstAction.Action;
        stAMA.ActionParam = rstAction.ActionParam;
        stAMA.Speed       = rstAction.Speed;
        stAMA.Direction   = rstAction.Direction;

        stAMA.X    = rstAction.X;
        stAMA.Y    = rstAction.Y;
        stAMA.EndX = rstAction.EndX;
        stAMA.EndY = rstAction.EndY;

        m_ActorPod->Forward({MPK_ACTION, stAMA}, m_Map->GetAddress());
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "can't dispatch action state");
    g_MonoServer->Restart();
}
