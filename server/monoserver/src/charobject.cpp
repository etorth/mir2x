/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 05/04/2017 00:59:49
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
    , m_MapCache()
    , m_CurrX(nMapX)
    , m_CurrY(nMapY)
    , m_Direction(nDirection)
    , m_FreezeMove(false)
    , m_Ability()
    , m_WAbility()
    , m_AddAbility()
{
    ResetState(STATE_LIFECYCLE, nLifeState);

    assert(m_Map);
    m_MapCache[m_Map->ID()] = m_Map;

    auto fnRegisterClass = [this]() -> void {
        if(!RegisterClass<CharObject, ActiveObject>()){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Class registration for <CharObject, ActiveObject> failed");
            g_MonoServer->Restart();
        }
    };
    static std::once_flag stFlag;
    std::call_once(stFlag, fnRegisterClass);
}

bool CharObject::NextLocation(int *pX, int *pY, int nDirection, int nDistance)
{
    int nDX = 0;
    int nDY = 0;
    switch(nDirection){
        case DIR_UP:        { nDX =  0; nDY = -1; break; }
        case DIR_UPRIGHT:   { nDX = +1; nDY = -1; break; }
        case DIR_RIGHT:     { nDX = +1; nDY =  0; break; }
        case DIR_DOWNRIGHT: { nDX = +1; nDY = +1; break; }
        case DIR_DOWN:      { nDX =  0; nDY = +1; break; }
        case DIR_DOWNLEFT:  { nDX = -1; nDY = +1; break; }
        case DIR_LEFT:      { nDX = -1; nDY =  0; break; }
        case DIR_UPLEFT:    { nDX = -1; nDY = -1; break; }
        default:            { return false;              }
    }

    if(pX){ *pX = m_CurrX + (nDX * nDistance); }
    if(pY){ *pY = m_CurrY + (nDY * nDistance); }

    return true;
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

bool CharObject::RequestMove(int nX, int nY, std::function<void()> fnOnMoveOK, std::function<void()> fnOnMoveError)
{
    if(true
            && CanMove()
            && m_Map->ValidC(nX, nY)){

        AMTryMove stAMTM;
        stAMTM.UID     = UID();
        stAMTM.MapID   = m_Map->ID();
        stAMTM.X       = X();
        stAMTM.Y       = Y();
        stAMTM.EndX    = nX;
        stAMTM.EndY    = nY;

        auto fnOP = [this, nX, nY, fnOnMoveOK, fnOnMoveError](const MessagePack &rstMPK, const Theron::Address &rstAddr){
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
                        if(fnOnMoveOK){ fnOnMoveOK(); }
                        m_FreezeMove = false;
                        break;
                    }
                case MPK_ERROR:
                    {
                        m_Direction = GetBack();
                        DispatchAction({ACTION_STAND, 0, m_Direction, X(), Y()});
                        if(fnOnMoveError){ fnOnMoveError(); }
                        m_FreezeMove = false;
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

        m_FreezeMove = true;
        return m_ActorPod->Forward({MPK_TRYMOVE, stAMTM}, m_Map->GetAddress(), fnOP);
    }
    return false;
}

bool CharObject::CanMove()
{
    return !m_FreezeMove;
}
