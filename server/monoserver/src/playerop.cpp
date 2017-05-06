/*
 * =====================================================================================
 *
 *       Filename: playerop.cpp
 *        Created: 05/11/2016 17:37:54
 *  Last Modified: 05/05/2017 18:18:39
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

#include <cinttypes>
#include "netpod.hpp"
#include "player.hpp"
#include "memorypn.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"

void Player::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    extern NetPodN *g_NetPodN;
    extern MonoServer *g_MonoServer;

    Update();

    SMPing stSMP;
    stSMP.Tick = g_MonoServer->GetTimeTick();
    g_NetPodN->Send(m_SessionID, SM_PING, stSMP);
}

void Player::On_MPK_BINDSESSION(const MessagePack &rstMPK, const Theron::Address &)
{
    Bind(*((uint32_t *)rstMPK.Data()));

    SMLoginOK stSMLOK;
    stSMLOK.UID       = UID();
    stSMLOK.DBID      = DBID();
    stSMLOK.MapID     = m_Map->ID();
    stSMLOK.X         = m_CurrX;
    stSMLOK.Y         = m_CurrY;
    stSMLOK.Male      = true;
    stSMLOK.Direction = m_Direction;
    stSMLOK.JobID     = m_JobID;
    stSMLOK.Level     = m_Level;

    extern NetPodN *g_NetPodN;
    g_NetPodN->Send(m_SessionID, SM_LOGINOK, stSMLOK);

    if(ActorPodValid() && m_Map->ActorPodValid()){
        AMPullCOInfo stAMPCOI;
        stAMPCOI.SessionID = m_SessionID;
        m_ActorPod->Forward({MPK_PULLCOINFO, stAMPCOI}, m_Map->GetAddress());
    }
}

void Player::On_MPK_NETPACKAGE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMNetPackage stAMNP;
    std::memcpy(&stAMNP, rstMPK.Data(), sizeof(AMNetPackage));

    OperateNet(stAMNP.Type, stAMNP.Data, stAMNP.DataLen);

    if(stAMNP.Data){
        extern MemoryPN *g_MemoryPN;
        g_MemoryPN->Free(const_cast<uint8_t *>(stAMNP.Data));
    }
}

void Player::On_MPK_ACTION(const MessagePack &rstMPK, const Theron::Address &)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if(stAMA.UID != UID()){
        if((std::abs(stAMA.X - m_CurrX) <= SYS_MAPVISIBLEW) && (std::abs(stAMA.Y - m_CurrY) <= SYS_MAPVISIBLEH)){
            SMAction stSMA;

            stSMA.UID   = stAMA.UID;
            stSMA.MapID = stAMA.MapID;

            stSMA.Action      = stAMA.Action;
            stSMA.ActionParam = stAMA.ActionParam;
            stSMA.Speed       = stAMA.Speed;
            stSMA.Direction   = stAMA.Direction;

            stSMA.X    = stAMA.X;
            stSMA.Y    = stAMA.Y;
            stSMA.EndX = stAMA.EndX;
            stSMA.EndY = stAMA.EndY;

            extern NetPodN *g_NetPodN;
            g_NetPodN->Send(m_SessionID, SM_ACTION, stSMA);
        }
    }
}

void Player::On_MPK_PULLCOINFO(const MessagePack &rstMPK, const Theron::Address &)
{
    AMPullCOInfo stAMPCOI;
    std::memcpy(&stAMPCOI, rstMPK.Data(), sizeof(stAMPCOI));
    if(stAMPCOI.SessionID != m_SessionID){
        ReportCORecord(stAMPCOI.SessionID);
    }
}

void Player::On_MPK_MAPSWITCH(const MessagePack &rstMPK, const Theron::Address &)
{
    AMMapSwitch stAMMS;
    std::memcpy(&stAMMS, rstMPK.Data(), sizeof(stAMMS));

    if(stAMMS.UID && stAMMS.MapID){
        extern MonoServer *g_MonoServer;
        if(auto stUIDRecord = g_MonoServer->GetUIDRecord(stAMMS.UID)){
            AMTryMapSwitch stAMTMS;
            stAMTMS.UID    = UID();
            stAMTMS.MapID  = m_Map->ID();
            stAMTMS.MapUID = m_Map->UID();

            stAMTMS.X = X();
            stAMTMS.Y = Y();

            // 1. send request to the new map
            //    if request rejected then it stays in current map
            auto fnOnResp = [this, stUIDRecord](const MessagePack &rstRMPK, const Theron::Address &){
                switch(rstRMPK.Type()){
                    case MPK_MAPSWITCHOK:
                        {
                            // new map permit this switch request
                            // new map will guarante to outlive current object
                            AMMapSwitchOK stAMMSOK;
                            std::memcpy(&stAMMSOK, rstRMPK.Data(), sizeof(stAMMSOK));
                            if(true
                                    && ((ServerMap *)(stAMMSOK.Data))
                                    && ((ServerMap *)(stAMMSOK.Data))->ID()
                                    && ((ServerMap *)(stAMMSOK.Data))->UID()
                                    && ((ServerMap *)(stAMMSOK.Data))->ValidC(stAMMSOK.X, stAMMSOK.Y)){

                                AMTryLeave stAMTL;
                                stAMTL.UID   = UID();
                                stAMTL.MapID = m_Map->ID();
                                stAMTL.X     = X();
                                stAMTL.Y     = Y();

                                // current map respond for the leave request
                                // dangerous here, we should keep m_Map always valid
                                auto fnOnLeaveResp = [this, stAMMSOK, rstRMPK](const MessagePack &rstLeaveRMPK, const Theron::Address &){
                                    switch(rstLeaveRMPK.Type()){
                                        case MPK_OK:
                                            {
                                                // 1. response to new map ``I am here"
                                                m_Map   = (ServerMap *)(stAMMSOK.Data);
                                                m_CurrX = stAMMSOK.X;
                                                m_CurrY = stAMMSOK.Y;
                                                m_ActorPod->Forward(MPK_OK, m_Map->GetAddress(), rstRMPK.ID());

                                                // 2. notify all players on the new map
                                                DispatchAction({ACTION_STAND, 0, Direction(), X(), Y(), m_Map->ID() });

                                                // 3. inform the client for map swith
                                                ReportStand();

                                                // 4. pull all co's on the new map
                                                AMPullCOInfo stAMPCOI;
                                                stAMPCOI.SessionID = m_SessionID;
                                                m_ActorPod->Forward({MPK_PULLCOINFO, stAMPCOI}, m_Map->GetAddress());

                                                break;
                                            }
                                        default:
                                            {
                                                // can't leave???, illegal response
                                                // server map won't respond any other message not MPK_OK
                                                // dangerous issue since we then can never inform the new map ``we can't come to you"
                                                m_ActorPod->Forward(MPK_ERROR, ((ServerMap *)(stAMMSOK.Data))->GetAddress(), rstRMPK.ID());

                                                extern MonoServer *g_MonoServer;
                                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Leave request failed: (UID = %" PRIu32 ", MapID = %" PRIu32 ")", UID(), ((ServerMap *)(stAMMSOK.Data))->ID());
                                                break;
                                            }
                                    }
                                };
                                m_ActorPod->Forward({MPK_TRYLEAVE, stAMTL}, m_Map->GetAddress(), fnOnLeaveResp);
                                return;
                            }

                            // AMMapSwitchOK invalid
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid AMMapSwitchOK: Map = %p", stAMMSOK.Data);
                            return;
                        }
                    default:
                        {
                            // do nothing
                            // new map reject this switch request
                            return;
                        }
                }
            };
            m_ActorPod->Forward({MPK_TRYMAPSWITCH, stAMTMS}, stUIDRecord.Address, fnOnResp);
            return;
        }
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "Map switch request failed: (UID = %" PRIu32 ", MapID = %" PRIu32 ")", stAMMS.UID, stAMMS.MapID);
}

void Player::On_MPK_QUERYLOCATION(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMLocation stAML;
    stAML.UID   = UID();
    stAML.MapID = MapID();
    stAML.X     = X();
    stAML.Y     = Y();

    m_ActorPod->Forward({MPK_LOCATION, stAML}, rstFromAddr, rstMPK.ID());
}
