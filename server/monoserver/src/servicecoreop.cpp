/*
 * =====================================================================================
 *
 *       Filename: servicecoreop.cpp
 *        Created: 05/03/2016 21:29:58
 *  Last Modified: 06/10/2016 23:40:28
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

    if(!ValidP(stAMACO.Common.MapID, stAMACO.Common.MapX, stAMACO.Common.MapY)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // TODO: should be very careful since this lambda will reply to the sender, if we firstly do a one-shoot
    //       try but not successful, so we reply as PENDING, then we install it as a trigger, when call it
    //       as a trigger, we reply PENDING again or ERROR / OK to the same MPK ID, then it immediately ran
    //       into trouble
    //
    auto fnDoAddCO = [this, stAMACO, rstFromAddr, nMPKID = rstMPK.ID(), bRespDone = false]() mutable -> bool {
        const auto &rstRMRecord = GetRMRecord(stAMACO.Common.MapID, stAMACO.Common.MapX, stAMACO.Common.MapY, false);
        switch(rstRMRecord.Query){
            case QUERY_NA:
                {
                    if(!bRespDone){
                        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
                        bRespDone = true;
                    }
                    return true;
                }
            case QUERY_OK:
                {
                    if(rstRMRecord.Valid()){
                        // TODO: here is communicate with rm so we can know the result exactly
                        //       but if currently it's QUERY_PENDING, the we have no idea
                        auto fnOnR = [stAMACO, this](const MessagePack &rstRMPK, const Theron::Address &){
                            switch(rstRMPK.Type()){
                                case MPK_OK:
                                    {
                                        extern MonoServer *g_MonoServer;
                                        g_MonoServer->AddLog(LOGTYPE_INFO, "add monster succeed: MonsterID = %d, MapID = %d, X = %d, Y = %d",
                                                stAMACO.Monster.MonsterID, stAMACO.Common.MapID, stAMACO.Common.MapX, stAMACO.Common.MapY);
                                        break;
                                    }
                                case MPK_ERROR:
                                    {
                                        extern MonoServer *g_MonoServer;
                                        g_MonoServer->AddLog(LOGTYPE_WARNING, "add monster failed: MonsterID = %d, MapID = %d, X = %d, Y = %d",
                                                stAMACO.Monster.MonsterID, stAMACO.Common.MapID, stAMACO.Common.MapX, stAMACO.Common.MapY);
                                        break;
                                    }
                                default:
                                    {
                                        extern MonoServer *g_MonoServer;
                                        g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported package type: %s", rstRMPK.Name());
                                        g_MonoServer->Restart();

                                        // to make the compiler happy
                                        break;
                                    }
                            }
                        };

                        m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, rstRMRecord.PodAddress, fnOnR);

                        // TODO: since here it's read state, could I put this response inside fnOnR? seems if I put inside, I
                        //       break the pending rule
                        //
                        //       A ask B to do C, and reply if C is succeed (OK) or not, to do C, B need to firstly do D, then
                        //       now, B may do D by itself, but:
                        //
                        //       0. do D and put C as a trigger when D is done, then when C is done reply OK
                        //       1. do D and put C as a trigger when D is done, reply PENDING immediately
                        //       2. just do D, but never touch C, and reply PENDING immediately
                        //       3. won't even touch C, and return PENDING immediately
                        //
                        //
                        //       0 and 3 are not acceptable, how about 1 and 2 ???
                        //
                        //       for action, currently I'm using 1, like here, if trying to adding a monster, we firstly need
                        //       to know the rm address of the destination. so we query the destination and put the adding as
                        //       a trigger, but we return PENDING immediately
                        //
                        //       same for query, if A ask B for information I, but B need to firstly ask C for I, then B may
                        //       ask C for I by itself after this query, but currently B just return PENDING
                        //
                        //
                        if(!bRespDone){
                            m_ActorPod->Forward(MPK_PENDING, rstFromAddr, nMPKID);
                            bRespDone = true;
                        }
                        return true;
                    }

                    // or it's a logic problem
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "unexpected logic error");
                    g_MonoServer->Restart();

                    // to make the compiler happy
                    return false;
                }
            case QUERY_PENDING:
                {
                    // ok the rm address is OTW but not valid now, we need to drive the trigger
                    // by ourself to make the add player command propagated since service core
                    // won't accept a metronome heart beat

                    extern EventTaskHub *g_EventTaskHub;
                    auto fnSendOneBeat = [stSCAddr = m_ActorPod->GetAddress()](){
                        SyncDriver().Forward(MPK_DUMMY, stSCAddr);
                    };

                    g_EventTaskHub->Add(200, fnSendOneBeat);

                    if(!bRespDone){
                        m_ActorPod->Forward(MPK_PENDING, rstFromAddr, nMPKID);
                        bRespDone = true;
                    }

                    // we still need to keep this trigger
                    // here is the only legal return with boolean false
                    //
                    // so with assistance of Install(), we can do loop query in the trigger
                    // 1. here send a heart beat to driver the trigger
                    // 2. outside one-shoot try to decide install this trigger
                    return false;
                }
            default:
                {
                    // for query state only have OK / NA / PENDING
                    // OK       : all good
                    // NA       : not a valid rm
                    // PENDING  : on the way
                    //
                    // we don't put an ERROR for rm address query result currently
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid query state for rm address");
                    g_MonoServer->Restart();

                    // to make the compiler happy
                    return false;
                }
        }

        // to make the compiler happy
        return false;
    };

    // do a one-shoot try, install trigger if failed
    if(!fnDoAddCO()){ m_StateHook.Install(fnDoAddCO); }
}

void ServiceCore::On_MPK_PLAYERPHATOM(const MessagePack &rstMPK, const Theron::Address &)
{
    AMPlayerPhantom stAMPP;
    std::memcpy(&stAMPP, rstMPK.Data(), sizeof(stAMPP));

    if(m_PlayerRecordMap.find(stAMPP.GUID) != m_PlayerRecordMap.end()){
    }
}

// don't try to find its sender, it's from a temp SyncDriver in the lambda
void ServiceCore::On_MPK_LOGINQUERYDB(const MessagePack &rstMPK, const Theron::Address &)
{
    AMLoginQueryDB stAMLQDB;
    std::memcpy(&stAMLQDB, rstMPK.Data(), sizeof(stAMLQDB));

    // TODO: check db record integration, if failed it's a big problem
    //       but this may be too strict since we may update the map, and if we reject like this
    //       the db could never be updated
    auto fnOnBadDBRecord = [stAMLQDB](){
        // extern MonoServer *g_MonoServer;
        // g_MonoServer->AddLog(LOGTYPE_WARNING, "bad db record: (map, x, y) = (%d, %d, %d)", stAMLQDB.MapID, stAMLQDB.MapX, stAMLQDB.MapY);
        // g_MonoServer->Restart();
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO, "rm is not capble on point (map, x, y) = (%d, %d, %d)", stAMLQDB.MapID, stAMLQDB.MapX, stAMLQDB.MapY);

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(stAMLQDB.SessionID, SM_LOGINFAIL, [nSID = stAMLQDB.SessionID](){g_NetPodN->Shutdown(nSID);});
    };

    if(!ValidP(stAMLQDB.MapID, stAMLQDB.MapX, stAMLQDB.MapY)){
        fnOnBadDBRecord();
        return;
    }

    auto fnUseDBRecord = [this, stAMLQDB, fnOnBadDBRecord]() -> bool{
        // 1. get the rm record
        const auto &rstRMRecord = GetRMRecord(stAMLQDB.MapID, stAMLQDB.MapX, stAMLQDB.MapY, false);

        // 2. check the rm query state
        switch(rstRMRecord.Query){
            case QUERY_NA:
                {
                    fnOnBadDBRecord();
                    return true;
                }
            case QUERY_OK:
                {
                    if(rstRMRecord.Valid()){
                        AMAddCharObject stAMACO;
                        stAMACO.Type = OBJECT_PLAYER;
                        stAMACO.Common.MapID     = stAMLQDB.MapID;
                        stAMACO.Common.MapX      = stAMLQDB.MapX;
                        stAMACO.Common.MapY      = stAMLQDB.MapY;
                        stAMACO.Common.R         = 10; // TODO
                        stAMACO.Player.GUID      = stAMLQDB.GUID;
                        stAMACO.Player.Level     = stAMLQDB.Level;
                        stAMACO.Player.JobID     = stAMLQDB.JobID;
                        stAMACO.Player.Direction = stAMLQDB.Direction;
                        stAMACO.Player.SessionID = stAMLQDB.SessionID;

                        // TODO: we put a callback here, for rm, MPK_ADDCHAROBJECT will be handled and return OK / ERROR
                        // state for succeed / failed to add the new char object
                        //
                        // but for add monster, since it's communicate with service core rather than rm, so it could have
                        // OK / ERROR / PENDING state
                        auto fnOnR = [stAMACO](const MessagePack &rstRMPK, const Theron::Address &){
                            switch(rstRMPK.Type()){
                                case MPK_OK:
                                    {
                                        extern MonoServer *g_MonoServer;
                                        g_MonoServer->AddLog(LOGTYPE_INFO, "add player succeed: GUID = %d, JobID = %d, MapID = %d, X = %d, Y = %d",
                                                stAMACO.Player.GUID, stAMACO.Player.JobID, stAMACO.Common.MapID, stAMACO.Common.MapX, stAMACO.Common.MapY);
                                        break;
                                    }
                                case MPK_ERROR:
                                    {
                                        extern MonoServer *g_MonoServer;
                                        g_MonoServer->AddLog(LOGTYPE_WARNING, "add player failed: GUID = %d, JobID = %d, MapID = %d, X = %d, Y = %d",
                                                stAMACO.Player.GUID, stAMACO.Player.JobID, stAMACO.Common.MapID, stAMACO.Common.MapX, stAMACO.Common.MapY);
                                        break;
                                    }
                                default:
                                    {
                                        extern MonoServer *g_MonoServer;
                                        g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported package type: %s", rstRMPK.Name());
                                        g_MonoServer->Restart();

                                        // to make the compiler happy
                                        break;
                                    }
                            }
                        };

                        m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, rstRMRecord.PodAddress, fnOnR);
                        return true;
                    }

                    // or it's a logic problem
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "unexpected logic error");
                    g_MonoServer->Restart();

                    // to make the compiler happy
                    return false;
                }
            case QUERY_PENDING:
                {
                    // ok the rm address is OTW but not valid now, we need to drive the trigger
                    // by ourself to make the add player command propagated since service core
                    // won't accept a metronome heart beat

                    extern EventTaskHub *g_EventTaskHub;
                    auto fnSendOneBeat = [stSCAddr = m_ActorPod->GetAddress()](){
                        SyncDriver().Forward(MPK_DUMMY, stSCAddr);
                    };

                    g_EventTaskHub->Add(200, fnSendOneBeat);

                    // we still need to keep this trigger
                    // here is the only legal return with boolean false
                    //
                    // so with assistance of Install(), we can do loop query in the trigger
                    // 1. here send a heart beat to driver the trigger
                    // 2. outside one-shoot try to decide install this trigger
                    return false;
                }
            default:
                {
                    // for query state only have OK / NA / PENDING
                    // OK       : all good
                    // NA       : not a valid rm
                    // PENDING  : on the way
                    //
                    // we don't put an ERROR for rm address query result currently
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid query state for rm address");
                    g_MonoServer->Restart();

                    // to make the compiler happy
                    return false;
                }
        }

        // to make the compiler happy
        return false;
    };

    // do a one-shoot try, install trigger if failed
    if(!fnUseDBRecord()){ m_StateHook.Install(fnUseDBRecord); }
}
