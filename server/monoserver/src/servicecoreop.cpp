/*
 * =====================================================================================
 *
 *       Filename: servicecoreop.cpp
 *        Created: 05/03/2016 21:29:58
 *  Last Modified: 03/27/2017 14:31:48
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

void ServiceCore::On_MPK_DUMMY(const MessagePack &, const Theron::Address &)
{
    // do nothing since this is message send by anyomous SyncDriver
    // to drive the anyomous trigger
}

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
    uint32_t nSessionID = *((uint32_t *)rstMPK.Data());
    if(nSessionID){
        extern NetPodN *g_NetPodN;
        g_NetPodN->Activate(nSessionID, GetAddress());
    }
}

void ServiceCore::On_MPK_ADDCHAROBJECT(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMAddCharObject stAMACO;
    std::memcpy(&stAMACO, rstMPK.Data(), sizeof(stAMACO));

    if(stAMACO.Common.MapID){
        auto pMap = RetrieveMap(stAMACO.Common.MapID);
        if(pMap && pMap->In(stAMACO.Common.MapID, stAMACO.Common.MapX, stAMACO.Common.MapY)){
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

    // invalid map id, report error
    m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
}

void ServiceCore::On_MPK_PLAYERPHATOM(const MessagePack &rstMPK, const Theron::Address &)
{
    AMPlayerPhantom stAMPP;
    std::memcpy(&stAMPP, rstMPK.Data(), sizeof(stAMPP));
}

// don't try to find its sender, it's from a temp SyncDriver in the lambda
void ServiceCore::On_MPK_LOGINQUERYDB(const MessagePack &rstMPK, const Theron::Address &)
{
    AMLoginQueryDB stAMLQDB;
    std::memcpy(&stAMLQDB, rstMPK.Data(), sizeof(stAMLQDB));

    // error handler when error happens
    auto fnOnBadDBRecord = [stAMLQDB](){
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
            stAMACO.Common.MapX      = stAMLQDB.MapX;
            stAMACO.Common.MapY      = stAMLQDB.MapY;
            stAMACO.Player.GUID      = stAMLQDB.GUID;
            stAMACO.Player.Level     = stAMLQDB.Level;
            stAMACO.Player.JobID     = stAMLQDB.JobID;
            stAMACO.Player.Direction = stAMLQDB.Direction;
            stAMACO.Player.SessionID = stAMLQDB.SessionID;

            auto fnOnR = [this, stAMACO, fnOnBadDBRecord, pMap](const MessagePack &rstRMPK, const Theron::Address &){
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

void ServiceCore::On_MPK_QUERYMONSTERGINFO(const MessagePack &rstMPK, const Theron::Address &)
{
    AMQueryMonsterGInfo stAMQMGI;
    std::memcpy(&stAMQMGI, rstMPK.Data(), sizeof(stAMQMGI));

    extern MonoServer *g_MonoServer;
    auto &rstRecord = g_MonoServer->MonsterGInfo(stAMQMGI.MonsterID); 

    if(rstRecord.Valid()){
        extern MemoryPN *g_MemoryPN;
        auto pBuf = (SMMonsterGInfo *)(g_MemoryPN->Get(sizeof(SMMonsterGInfo)));

        pBuf->MonsterID = stAMQMGI.MonsterID;
        pBuf->LookIDN   = stAMQMGI.LookIDN;
        pBuf->LookID   = rstRecord.LookID((int)stAMQMGI.LookIDN);

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(stAMQMGI.SessionID, SM_MONSTERGINFO, (uint8_t *)pBuf, sizeof(SMMonsterGInfo), [pBuf](){ g_MemoryPN->Free(pBuf); });
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_FATAL, "Query monster global information failed");
    g_MonoServer->Restart();
}
