/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 05/06/2017 18:16:40
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
    , m_TargetInfo()
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
            static const int nDirV[][3] = {
                {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
                {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
                {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

            int nDX = nX - m_CurrX + 1;
            int nDY = nY - m_CurrY + 1;

            int nNewDirection = nDirV[nDY][nDX];

            switch(rstMPK.Type()){
                case MPK_OK:
                    {
                        int nOldX = X();
                        int nOldY = Y();

                        m_CurrX     = nX;
                        m_CurrY     = nY;
                        m_Direction = nNewDirection;

                        m_ActorPod->Forward(MPK_OK, rstAddr, rstMPK.ID());
                        DispatchAction({ACTION_MOVE, 0, 1, Direction(), nOldX, nOldY, X(), Y(), m_Map->ID()});

                        if(fnOnMoveOK){ fnOnMoveOK(); }
                        m_FreezeMove = false;
                        break;
                    }
                case MPK_ERROR:
                    {
                        m_Direction = nNewDirection;
                        DispatchAction({ACTION_STAND, 0, Direction(), X(), Y(), m_Map->ID()});

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

bool CharObject::TrackTarget()
{
    if(CanMove()){
        if(m_TargetInfo.UID){
            extern MonoServer *g_MonoServer;
            if(auto stRecord = g_MonoServer->GetUIDRecord(m_TargetInfo.UID)){
                AMQueryLocation stAMQL;
                stAMQL.UID   = UID();
                stAMQL.MapID = MapID();

                auto fnOnResp = [this](const MessagePack &rstRMPK, const Theron::Address &rstFromAddr){
                    switch(rstRMPK.Type()){
                        case MPK_LOCATION:
                            {
                                AMLocation stAML;
                                std::memcpy(&stAML, rstRMPK.Data(), sizeof(stAML));

                                if(stAML.MapID == MapID()){
                                    m_TargetInfo.MapID = stAML.MapID;
                                    m_TargetInfo.X     = stAML.X;
                                    m_TargetInfo.Y     = stAML.Y;

                                    switch(LDistance2(m_TargetInfo.X, m_TargetInfo.Y, X(), Y())){
                                        case 0:
                                            {
                                                break;
                                            }
                                        case 1:
                                        case 2:
                                            {
                                                static const int nDirV[][3] = {
                                                    {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
                                                    {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
                                                    {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

                                                int nDX = m_TargetInfo.X - X() + 1;
                                                int nDY = m_TargetInfo.Y - Y() + 1;

                                                if(Direction() != nDirV[nDY][nDX]){
                                                    m_Direction = nDirV[nDY][nDX];
                                                }

                                                // 1. dispatch action to all
                                                DispatchAction({ACTION_ATTACK, 0, Direction(), X(), Y(), m_Map->ID()});

                                                // 2. send attack message to target
                                                //    target can ignore this message directly
                                                AMAttack stAMA;
                                                stAMA.UID   = UID();
                                                stAMA.MapID = MapID();

                                                stAMA.X = X();
                                                stAMA.Y = Y();
                                                m_ActorPod->Forward({MPK_ATTACK, stAMA}, rstFromAddr);
                                                break;
                                            }
                                        default:
                                            {
                                                // find a path and ignore the objects on the way
                                                // if not ignore, we use ServerMap::CanMove() instead of GroundValid()
                                                // this makes the path finding always fail since the start and end point has been taken
                                                AMPathFind stAMPF;
                                                stAMPF.UID     = UID();
                                                stAMPF.MapID   = MapID();
                                                stAMPF.CheckCO = true;
                                                stAMPF.X       = X();
                                                stAMPF.Y       = Y();
                                                stAMPF.EndX    = m_TargetInfo.X;
                                                stAMPF.EndY    = m_TargetInfo.Y;

                                                auto fnOnResp = [this](const MessagePack &rstRMPK, const Theron::Address &){
                                                    switch(rstRMPK.Type()){
                                                        case MPK_PATHFINDOK:
                                                            {
                                                                AMPathFindOK stAMPFOK;
                                                                std::memcpy(&stAMPFOK, rstRMPK.Data(), sizeof(stAMPFOK));
                                                                RequestMove(stAMPFOK.Point[1].X, stAMPFOK.Point[1].Y, [](){}, [](){});
                                                                break;
                                                            }
                                                        default:
                                                            {
                                                                break;
                                                            }
                                                    }
                                                };
                                                m_ActorPod->Forward({MPK_PATHFIND, stAMPF}, m_Map->GetAddress(), fnOnResp);
                                                break;
                                            }
                                    }
                                }else{
                                    m_TargetInfo.UID = 0;
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                };

                // if we have an target follow the target only
                m_ActorPod->Forward({MPK_QUERYLOCATION, stAMQL}, stRecord.Address, fnOnResp);
                return true;
            }

            // else it's not valid any more
            m_TargetInfo.UID = 0;
        }
    }
    return false;
}
