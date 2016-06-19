/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 06/19/2016 11:53:01
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

CharObject::CharObject()
    : ActiveObject()
    , m_EmptyAddress(Theron::Address::Null())
    , m_RMAddress(Theron::Address::Null())
    , m_MapAddress(Theron::Address::Null())
    , m_SCAddress(Theron::Address::Null())
    , m_MapAddressQuery(QUERY_NA)
    , m_SCAddressQuery(QUERY_NA)
    , m_MapID(0)
    , m_CurrX(0)
    , m_CurrY(0)
    , m_R(0)
    , m_Direction(DIR_UP)
    , m_Name("")
{}

CharObject::~CharObject()
{}

void CharObject::NextLocation(int *pX, int *pY, int nDistance)
{
    double fDX[] = {+0.000, +0.707, +1.000, +0.707, +0.000, -0.707, -1.000, -0.707};
    double fDY[] = {-1.000, -0.707, +0.000, +0.707, +1.000, +0.707, +0.000, -0.707};

    if(pX){ *pX = m_CurrX + std::lround(fDX[m_Direction] * nDistance); }
    if(pY){ *pY = m_CurrY + std::lround(fDY[m_Direction] * nDistance); }
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
    if(!ActorPodValid()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "action dispatching require actor to be activated");
        g_MonoServer->Restart();
    }

    switch(QueryMapAddress()){
        case QUERY_OK:
            {
                if(m_MapAddress){
                    AMActionState stAMAS;
                    stAMAS.UID     = UID();
                    stAMAS.AddTime = AddTime();
                    stAMAS.Speed   = Speed();

                    stAMAS.X     = X();
                    stAMAS.Y     = Y();
                    stAMAS.R     = R();
                    stAMAS.MapID = MapID();

                    stAMAS.Action    = (uint32_t)Action();
                    stAMAS.Direction = Direction();

                    m_ActorPod->Forward({MPK_ACTIONSTATE, stAMAS}, m_MapAddress);
                    return;
                }

                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "internal logic error");
                g_MonoServer->Restart();
                break;
            }
        case QUERY_PENDING:
            {
                // TODO: currently I just ignore this motion
                //       since it's just a state boardcat, no need to install a trigger
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unexpected state for map address query");
                g_MonoServer->Restart();
                break;
            }
    }
}

int CharObject::QuerySCAddress()
{
    switch(m_SCAddressQuery){
        case QUERY_OK:
            {
                return QUERY_OK;
            }
        case QUERY_PENDING:
            {
                return QUERY_PENDING;
            }
        case QUERY_NA:
            {
                // 1. check state for currnet actor
                if(!ActorPodValid()){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "try to send message before activated");
                    g_MonoServer->Restart();
                }

                if(!m_RMAddress){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "activated char object works without valid rm address");
                    g_MonoServer->Restart();
                }

                // 2. ask for service core address
                auto fnOnR = [this](const MessagePack &rstRMPK, const Theron::Address &){
                    switch(rstRMPK.Type()){
                        case MPK_ADDRESS:
                            {
                                m_SCAddress = Theron::Address((const char *)rstRMPK.Data());
                                m_SCAddressQuery = QUERY_OK;
                                break;
                            }
                        case MPK_PENDING:
                            {
                                // ask regionmonitor for service core address but the map said it's not ready currently, we
                                // need to ask again if we need it, so we have to put NA to m_SCAddressQuery
                                //
                                // TODO do I need to tolerate the PENDING here?
                                //      previously I take PENDING of sc address query as serious error

                                // 1. put NA to m_SCAddressQuery to ask agin for QuerySCAddress()
                                m_SCAddressQuery = QUERY_NA;

                                // 2. query again, we have two ways to ask
                                //      1. directly ask for it
                                //          QuerySCAddress();
                                //      2. put it in the trigger
                                //          m_StateHook.Install([this](){
                                //              return QuerySCAddress() == QUERY_OK;
                                //          });
                                //
                                //    but don't use EventTaskHub to ask since it interfer the interal state
                                //          g_EventTaskHub->Add(200, [this](){ QuerySCAddress(); });
                                //
                                //    method-1 is better since it only repeat on regionmonitor's response, method-2 will be
                                //    triggered every time the actor got a message
                                //
                                QuerySCAddress();
                                break;
                            }
                        default:
                            {
                                // sc address query failed, couldn't happen
                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "query service core address failed, couldn't happen");
                                g_MonoServer->Restart();
                                break;
                            }
                    }
                };

                m_ActorPod->Forward(MPK_QUERYSCADDRESS, m_RMAddress, fnOnR);
                m_SCAddressQuery = QUERY_PENDING;

                // we are still pending now
                return QUERY_PENDING;
            }
        default:
            {
                // otherwise it's should be an error
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "activated RM works without valid map address");
                g_MonoServer->Restart();
                return QUERY_ERROR;
            }
    }

    // to make the compiler happy
    return QUERY_ERROR;
}

int CharObject::QueryMapAddress()
{
    switch(m_MapAddressQuery){
        case QUERY_OK:
            {
                return QUERY_OK;
            }
        case QUERY_PENDING:
            {
                return QUERY_PENDING;
            }
        case QUERY_NA:
            {
                // 1. check state for currnet actor
                if(!ActorPodValid()){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "try to send message before activated");
                    g_MonoServer->Restart();
                }

                if(!m_RMAddress){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "activated char object works without valid rm address");
                    g_MonoServer->Restart();
                }

                // 2. ask for service core address
                auto fnOnR = [this](const MessagePack &rstRMPK, const Theron::Address &){
                    switch(rstRMPK.Type()){
                        case MPK_ADDRESS:
                            {
                                m_MapAddress = Theron::Address((const char *)rstRMPK.Data());
                                m_MapAddressQuery = QUERY_OK;
                                break;
                            }
                        case MPK_PENDING:
                            {
                                // ask regionmonitor for map address but the rm said it's not ready currently, we
                                // need to ask again if we need it, so we have to put NA to m_MapAddressQuery
                                //
                                // TODO do I need to tolerate the PENDING here?
                                //      because rm is supposed to be ready always for map address query

                                // 1. put NA to m_MapAddressQuery to ask agin for QueryMapAddress()
                                m_MapAddressQuery = QUERY_NA;

                                // 2. query again, we have two ways to ask
                                //      1. directly ask for it
                                //          QueryMapAddress();
                                //      2. put it in the trigger
                                //          m_StateHook.Install([this](){
                                //              return QueryMapAddress() == QUERY_OK;
                                //          });
                                //
                                //    but don't use EventTaskHub to ask since it interfer the interal state
                                //          g_EventTaskHub->Add(200, [this](){ QueryMapAddress(); });
                                //
                                //    method-1 is better since it only repeat on regionmonitor's response, method-2 will be
                                //    triggered every time the actor got a message
                                //
                                QueryMapAddress();
                                break;
                            }
                        default:
                            {
                                // sc address query failed, couldn't happen
                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "query map address failed, couldn't happen");
                                g_MonoServer->Restart();
                                break;
                            }
                    }
                };

                m_ActorPod->Forward({MPK_QUERYMAPADDRESS, m_MapID}, m_RMAddress, fnOnR);
                m_MapAddressQuery = QUERY_PENDING;

                // we are still pending now
                return QUERY_PENDING;
            }
        default:
            {
                // otherwise it's should be an error
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "activated RM works without valid map address");
                g_MonoServer->Restart();
                return QUERY_ERROR;
            }
    }

    // to make the compiler happy
    return QUERY_ERROR;
}

int CharObject::Action()
{
    switch(State(STATE_ACTION)){
        case STATE_ACTION:
            {
                // undefined
                return ACTION_UNKNOWN;
            }
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

    // to make the compiler happy
    return ACTION_UNKNOWN;
}
