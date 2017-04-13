/*
 * =====================================================================================
 *
 *       Filename: servermapop.cpp
 *        Created: 05/03/2016 20:21:32
 *  Last Modified: 04/13/2017 01:10:23
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

#include "player.hpp"
#include "monster.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "actorpod.hpp"
#include "metronome.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"

void ServerMap::On_MPK_HI(const MessagePack &, const Theron::Address &)
{
    delete m_Metronome;
    m_Metronome = new Metronome(1000);
    m_Metronome->Activate(GetAddress());
}

void ServerMap::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    for(auto &rstRecordLine: m_ObjectV2D){
        for(auto &rstRecordV: rstRecordLine){
            for(auto pObject: rstRecordV){
                if(pObject && pObject->Active()){
                    m_ActorPod->Forward(MPK_METRONOME, ((ActiveObject *)(pObject))->GetAddress());
                }
            }
        }
    }
}

void ServerMap::On_MPK_ACTION(const MessagePack &rstMPK, const Theron::Address &)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if(ValidC(stAMA.X, stAMA.Y)){
        auto nX0 = std::max<int>(0,   (stAMA.X - SYS_MAPVISIBLEW));
        auto nY0 = std::max<int>(0,   (stAMA.Y - SYS_MAPVISIBLEH));
        auto nX1 = std::min<int>(W(), (stAMA.X + SYS_MAPVISIBLEW));
        auto nY1 = std::min<int>(H(), (stAMA.Y + SYS_MAPVISIBLEH));

        for(int nX = nX0; nX <= nX1; ++nX){
            for(int nY = nY0; nY <= nY1; ++nY){
                if(ValidC(nX, nY)){
                    for(auto pObject: m_ObjectV2D[nX][nY]){
                        if(pObject && pObject->Active()){
                            if(((ActiveObject *)(pObject))->Type(TYPE_HUMAN)){
                                m_ActorPod->Forward({MPK_ACTION, stAMA}, ((ActiveObject *)(pObject))->GetAddress());
                            }
                        }
                    }
                }
            }
        }
    }
}

void ServerMap::On_MPK_ADDCHAROBJECT(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMAddCharObject stAMACO;
    std::memcpy(&stAMACO, rstMPK.Data(), sizeof(stAMACO));

    if(!In(stAMACO.Common.MapID, stAMACO.Common.MapX, stAMACO.Common.MapY)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument in adding char object package");
        g_MonoServer->Restart();
    }

    if(!CanMove(stAMACO.Common.MapX, stAMACO.Common.MapY)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    if(m_CellStateV2D[stAMACO.Common.MapX][stAMACO.Common.MapY].Freezed){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    switch(stAMACO.Type){
        case TYPE_MONSTER:
            {
                auto pCO = new Monster(stAMACO.Monster.MonsterID,
                        m_ServiceCore,
                        this,
                        stAMACO.Common.MapX,
                        stAMACO.Common.MapY,
                        0,
                        STATE_INCARNATED);
                m_ObjectV2D[stAMACO.Common.MapX][stAMACO.Common.MapY].push_back(pCO);
                pCO->Activate();
                m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                break;
            }
        case TYPE_PLAYER:
            {
                auto pCO = new Player(stAMACO.Player.GUID,
                        stAMACO.Player.JobID,
                        0,
                        m_ServiceCore,
                        this,
                        stAMACO.Common.MapX,
                        stAMACO.Common.MapY,
                        0,
                        STATE_INCARNATED);
                m_ObjectV2D[stAMACO.Common.MapX][stAMACO.Common.MapY].push_back(pCO);
                pCO->Activate();
                m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                m_ActorPod->Forward({MPK_BINDSESSION, stAMACO.Player.SessionID}, pCO->GetAddress());
                break;
            }
        default:
            {
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                break;
            }
    }
}

void ServerMap::On_MPK_TRYSPACEMOVE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMTrySpaceMove stAMTSM;
    std::memcpy(&stAMTSM, rstMPK.Data(), sizeof(stAMTSM));

    if(!In(stAMTSM.MapID, stAMTSM.X, stAMTSM.Y)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "destination is not in current map, routing error");
        g_MonoServer->Restart();
    }
}

void ServerMap::On_MPK_TRYMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    if(!In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "destination is not in current map, routing error");
        g_MonoServer->Restart();
    }

    if(!CanMove(stAMTM.X, stAMTM.Y)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    if(m_CellStateV2D[stAMTM.X][stAMTM.Y].Freezed){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    auto fnOnR = [this, stAMTM](const MessagePack &rstRMPK, const Theron::Address &){
        switch(rstRMPK.Type()){
            case MPK_OK:
                {
                    // the object comfirm to move
                    // and it's internal state has changed

                    // 1. leave last cell
                    {
                        bool bFind = false;
                        auto &rstObjectV = m_ObjectV2D[stAMTM.CurrX][stAMTM.CurrY];

                        for(auto &pObject: rstObjectV){
                            if((void *)(pObject) == stAMTM.This){
                                // 1. mark as find
                                bFind = true;

                                // 2. remove from the object list
                                std::swap(pObject, rstObjectV.back());
                                rstObjectV.pop_back();

                                break;
                            }
                        }

                        if(!bFind){
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_FATAL, "char object %p is not in current map", stAMTM.This);
                            g_MonoServer->Restart();
                        }
                    }

                    // 2. push it to the new cell
                    m_ObjectV2D[stAMTM.X][stAMTM.Y].push_back((ServerObject *)(stAMTM.This));
                    break;
                }
            default:
                {
                    break;
                }
        }
        m_CellStateV2D[stAMTM.X][stAMTM.Y].Freezed = false;
    };

    m_CellStateV2D[stAMTM.X][stAMTM.Y].Freezed = true;
    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnOnR);
}

void ServerMap::On_MPK_LEAVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMLeave stAML;
    std::memcpy(&stAML, rstMPK.Data(), rstMPK.DataLen());

    bool bFind = false;
    auto &rstObjectV = m_ObjectV2D[stAML.X][stAML.Y];

    for(auto &pObject: rstObjectV){
        if((void *)(pObject) == stAML.This){
            // 1. mark as find
            bFind = true;

            // 2. remove from the object list
            std::swap(rstObjectV.back(), pObject);
            rstObjectV.pop_back();

            // 3. inform the co that now you can leave
            m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
            break;
        }
    }

    if(!bFind){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "char object %p is not in current map", stAML.This);
        g_MonoServer->Restart();
    }
}

void ServerMap::On_MPK_PULLCOINFO(const MessagePack &rstMPK, const Theron::Address &)
{
    AMPullCOInfo stAMPCOI;
    std::memcpy(&stAMPCOI, rstMPK.Data(), sizeof(stAMPCOI));

    for(auto &rstRecordLine: m_ObjectV2D){
        for(auto &rstRecordV: rstRecordLine){
            for(auto pObject: rstRecordV){
                if(pObject && pObject->Active()){
                    m_ActorPod->Forward({MPK_PULLCOINFO, stAMPCOI.SessionID}, ((ActiveObject *)(pObject))->GetAddress());
                }
            }
        }
    }
}
