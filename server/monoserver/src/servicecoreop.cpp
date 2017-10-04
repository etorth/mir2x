/*
 * =====================================================================================
 *
 *       Filename: servicecoreop.cpp
 *        Created: 05/03/2016 21:29:58
 *  Last Modified: 10/03/2017 22:18:57
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
#include <string>

#include "player.hpp"
#include "memorypn.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"

// ServiceCore accepts net packages from *many* sessions and based on it to create
// the player object for a one to one map
//
// So servicecore <-> session is 1 to N, means we have to put put pointer of session
// in the net package otherwise we can't find the session even we have session's 
// address, session is a sync-driver, even we have it's address we can't find it
//
void ServiceCore::On_MPK_NETPACKAGE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMNetPackage stAMNP;
    std::memcpy(&stAMNP, rstMPK.Data(), sizeof(stAMNP));

    OperateNet(stAMNP.SessionID, stAMNP.Type, stAMNP.Data, stAMNP.DataLen);
}

// TODO
// based on the information of coming connection
// do IP banning etc. currently only activate this session
void ServiceCore::On_MPK_NEWCONNECTION(const MessagePack &rstMPK, const Theron::Address &)
{
    AMNewConnection stAMNC;
    std::memcpy(&stAMNC, rstMPK.Data(), sizeof(stAMNC));

    if(stAMNC.SessionID){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO, "ServiceCore get informed for new connection: %d", (int)(stAMNC.SessionID));

        extern NetPodN *g_NetPodN;
        g_NetPodN->Activate(stAMNC.SessionID, GetAddress());
    }
}

void ServiceCore::On_MPK_ADDCHAROBJECT(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMAddCharObject stAMACO;
    std::memcpy(&stAMACO, rstMPK.Data(), sizeof(stAMACO));

    if(stAMACO.Common.MapID){
        if(auto pMap = RetrieveMap(stAMACO.Common.MapID)){
            if(false
                    || stAMACO.Common.Random
                    || pMap->In(stAMACO.Common.MapID, stAMACO.Common.X, stAMACO.Common.Y)){

                auto fnOP = [this, stAMACO, rstMPK, rstFromAddr](const MessagePack &rstRMPK, const Theron::Address &){
                    switch(rstRMPK.Type()){
                        case MPK_OK:
                            {
                                m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                                break;
                            }
                        default:
                            {
                                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                                break;
                            }
                    }
                };
                m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, pMap->GetAddress(), fnOP);
                return;
            }
        }
    }

    // invalid location info, return error directly
    m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
}

// don't try to find its sender, it's from a temp SyncDriver in the lambda
void ServiceCore::On_MPK_LOGINQUERYDB(const MessagePack &rstMPK, const Theron::Address &)
{
    AMLoginQueryDB stAMLQDB;
    std::memcpy(&stAMLQDB, rstMPK.Data(), sizeof(stAMLQDB));

    // error handler when error happens
    auto fnOnBadDBRecord = [stAMLQDB]()
    {
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO, "Login failed for (%d, %d, %d)", stAMLQDB.MapID, stAMLQDB.MapX, stAMLQDB.MapY);

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(stAMLQDB.SessionID, SM_LOGINFAIL, [nSID = stAMLQDB.SessionID](){g_NetPodN->Shutdown(nSID);});
    };

    if(stAMLQDB.MapID){
        auto pMap = m_MapRecord.find(stAMLQDB.MapID);
        if(pMap == m_MapRecord.end()){
            if(!LoadMap(stAMLQDB.MapID)){
                fnOnBadDBRecord();
                return;
            }
            pMap = m_MapRecord.find(stAMLQDB.MapID);
        }

        if((pMap == m_MapRecord.end()) || (pMap->second == nullptr) || (pMap->second->ID() != stAMLQDB.MapID)){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_FATAL, "internal logic error");
            g_MonoServer->Restart();
        }

        if(pMap->second->In(stAMLQDB.MapID, stAMLQDB.MapX, stAMLQDB.MapY)){
            AMAddCharObject stAMACO;
            stAMACO.Type = TYPE_PLAYER;
            stAMACO.Common.MapID     = stAMLQDB.MapID;
            stAMACO.Common.X         = stAMLQDB.MapX;
            stAMACO.Common.Y         = stAMLQDB.MapY;
            stAMACO.Common.Random    = true;
            stAMACO.Player.DBID      = stAMLQDB.DBID;
            stAMACO.Player.Level     = stAMLQDB.Level;
            stAMACO.Player.JobID     = stAMLQDB.JobID;
            stAMACO.Player.Direction = stAMLQDB.Direction;
            stAMACO.Player.SessionID = stAMLQDB.SessionID;

            auto fnOnR = [this, stAMACO, fnOnBadDBRecord, pMap](const MessagePack &rstRMPK, const Theron::Address &)
            {
                switch(rstRMPK.Type()){
                    case MPK_OK:
                        {
                            break;
                        }
                    default:
                        {
                            fnOnBadDBRecord();
                            break;
                        }
                }
            };

            m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, pMap->second->GetAddress(), fnOnR);
            return;
        }
    }

    fnOnBadDBRecord();
}

void ServiceCore::On_MPK_QUERYMAPLIST(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMMapList stAMML;
    std::memset(&stAMML, 0, sizeof(stAMML));

    size_t nIndex = 0;
    for(auto pMap: m_MapRecord){
        if(pMap.second && pMap.second->ID()){
            if(nIndex < (sizeof(stAMML.MapList) / sizeof(stAMML.MapList[0]))){
                stAMML.MapList[nIndex++] = pMap.second->ID();
            }else{
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_FATAL, "Need larger map list size in AMMapList");
                g_MonoServer->Restart();
            }
        }
    }
    m_ActorPod->Forward({MPK_MAPLIST, stAMML}, rstFromAddr, rstMPK.ID());
}

void ServiceCore::On_MPK_TRYMAPSWITCH(const MessagePack &rstMPK, const Theron::Address &)
{
    AMTryMapSwitch stAMTMS;
    std::memcpy(&stAMTMS, rstMPK.Data(), sizeof(stAMTMS));

    if(stAMTMS.MapID){
        if(auto pMap = RetrieveMap(stAMTMS.MapID)){
            m_ActorPod->Forward({MPK_TRYMAPSWITCH, stAMTMS}, pMap->GetAddress());
        }
    }
}

void ServiceCore::On_MPK_QUERYMAPUID(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMQueryMapUID stAMQMUID;
    std::memcpy(&stAMQMUID, rstMPK.Data(), sizeof(stAMQMUID));

    if(auto pMap = RetrieveMap(stAMQMUID.MapID)){
        AMUID stAMUID;
        stAMUID.UID = pMap->UID();
        m_ActorPod->Forward({MPK_UID, stAMUID}, rstFromAddr, rstMPK.ID());
    }else{
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    }
}

void ServiceCore::On_MPK_QUERYCOCOUNT(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMQueryCOCount stAMQCOC;
    std::memcpy(&stAMQCOC, rstMPK.Data(), sizeof(stAMQCOC));

    int nCheckCount = 0;
    if(stAMQCOC.MapID){
        if(m_MapRecord.find(stAMQCOC.MapID) == m_MapRecord.end()){
            nCheckCount = 0;
        }else{
            nCheckCount = 1;
        }
    }else{
        nCheckCount = (int)(m_MapRecord.size());
    }

    switch(nCheckCount){
        case 0:
            {
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                return;
            }
        case 1:
            {
                if(auto pMap = (stAMQCOC.MapID ? m_MapRecord[stAMQCOC.MapID] : m_MapRecord.begin()->second)){
                    auto fnOnResp = [this, rstMPK, rstFromAddr](const MessagePack &rstRMPK, const Theron::Address &)
                    {
                        switch(rstRMPK.Type()){
                            case MPK_COCOUNT:
                                {
                                    m_ActorPod->Forward({MPK_COCOUNT, rstRMPK.Data(), rstRMPK.DataLen()}, rstFromAddr, rstMPK.ID());
                                    return;
                                }
                            case MPK_ERROR:
                            default:
                                {
                                    m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                                    return;
                                }
                        }
                    };
                    m_ActorPod->Forward({MPK_QUERYCOCOUNT, stAMQCOC}, pMap->GetAddress(), fnOnResp);
                    return;
                }else{
                    m_MapRecord.erase(stAMQCOC.MapID);
                    m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                    return;
                }
            }
        default:
            {
                // difficult part
                // need send multiple query message and collect them
                // after all collected we need to return the sum, problem:
                // 1. share state
                // 2. error handle

                struct SharedState
                {
                    bool Done;
                    int  CheckCount;
                    int  COCount;

                    SharedState(int nCheckCount)
                        : Done(false)
                        , CheckCount(nCheckCount)
                        , COCount(0)
                    {}
                };

                // current I don't have error handling
                // means if one query didn't get responded it will wait forever
                // to solve this issue, we can install an state hook but for simplity not now

                auto pSharedState = std::make_shared<SharedState>(nCheckCount);
                auto fnOnResp = [pSharedState, this, rstFromAddr, rstMPK](const MessagePack &rstRMPK, const Theron::Address &)
                {
                    switch(rstRMPK.Type()){
                        case MPK_COCOUNT:
                            {
                                if(pSharedState->Done){
                                    // we get response but shared state shows ``done"
                                    // means more than one error has alreay happened before
                                    // do nothing
                                }else{
                                    // get one more valid response
                                    // need to check if we need to response to sender
                                    AMCOCount stAMCOC;
                                    std::memcpy(&stAMCOC, rstRMPK.Data(), sizeof(stAMCOC));

                                    if(pSharedState->CheckCount == 1){
                                        stAMCOC.Count += pSharedState->COCount;
                                        m_ActorPod->Forward({MPK_COCOUNT, stAMCOC}, rstFromAddr, rstMPK.ID());
                                    }else{
                                        pSharedState->CheckCount--;
                                        pSharedState->COCount += (int)(stAMCOC.Count);
                                    }
                                }
                                return;
                            }
                        case MPK_ERROR:
                        default:
                            {
                                if(pSharedState->Done){
                                    // we get response but shared state shows ``done"
                                    // means more than one error has alreay happened before
                                    // do nothing
                                }else{
                                    // get first error
                                    m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                                }
                                return;
                            }
                    }
                };

                for(auto pRecord: m_MapRecord){
                    m_ActorPod->Forward({MPK_QUERYCOCOUNT, stAMQCOC}, pRecord.second->GetAddress(), fnOnResp);
                }
                return;
            }
    }
}
