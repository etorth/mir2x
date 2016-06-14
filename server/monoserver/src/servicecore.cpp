/*
 * =====================================================================================
 *
 *       Filename: servicecore.cpp
 *        Created: 04/22/2016 18:16:53
 *  Last Modified: 06/13/2016 23:10:17
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

#include <cstring>
#include <system_error>

#include "player.hpp"
#include "actorpod.hpp"
#include "metronome.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"

static int s_Count = 0;

ServiceCore::ServiceCore()
    : Transponder()
    , m_EmptyRMRecord()
{
    s_Count++;
    if(s_Count > 1){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "one service core please");
        g_MonoServer->Restart();

        // no use just put it here
        throw std::error_code();
    }
}

ServiceCore::~ServiceCore()
{
    s_Count--;
}

void ServiceCore::Operate(const MessagePack &rstMPK, const Theron::Address &rstAddr)
{
    switch(rstMPK.Type()){
        case MPK_DUMMY:
            {
                On_MPK_DUMMY(rstMPK, rstAddr);
                break;
            }
        case MPK_LOGINQUERYDB:
            {
                On_MPK_LOGINQUERYDB(rstMPK, rstAddr);
                break;
            }
        case MPK_ADDCHAROBJECT:
            {
                On_MPK_ADDCHAROBJECT(rstMPK, rstAddr);
                break;
            }
        case MPK_NEWCONNECTION:
            {
                On_MPK_NEWCONNECTION(rstMPK, rstAddr);
                break;
            }
        case MPK_NETPACKAGE:
            {
                On_MPK_NETPACKAGE(rstMPK, rstAddr);
                break;
            }
        case MPK_QUERYMONSTERGINFO:
            {
                On_MPK_QUERYMONSTERGINFO(rstMPK, rstAddr);
                break;
            }
        case MPK_PLAYERPHATOM:
            {
                On_MPK_PLAYERPHATOM(rstMPK, rstAddr);
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported message: %s", rstMPK.Name());
                g_MonoServer->Restart();
                break;
            }
    }
}

void ServiceCore::OperateNet(uint32_t nSID, uint8_t nType, const uint8_t *pData, size_t nDataLen)
{
    switch(nType){
        case CM_LOGIN:
            {
                Net_CM_Login(nSID, nType, pData, nDataLen);
                break;
            }
        default:
            {
                break;
            }
    }
}

bool ServiceCore::LoadMap(uint32_t nMapID)
{
    if(nMapID == 0){ return false; }
    if(m_MapRecordMap.find(nMapID) != m_MapRecordMap.end()){ return false; }

    ServerMap *pNewMap = new ServerMap(nMapID);
    auto &rstMapRecord = m_MapRecordMap[nMapID];

    rstMapRecord.MapID      = nMapID;
    rstMapRecord.Map        = pNewMap;
    rstMapRecord.GridW      = pNewMap->W();
    rstMapRecord.GridH      = pNewMap->H();
    rstMapRecord.RMW        = 1;
    rstMapRecord.RMH        = 1;
    rstMapRecord.PodAddress = pNewMap->Activate();

    m_ActorPod->Forward(MPK_HI, rstMapRecord.PodAddress);

    return true;
}

bool ServiceCore::ValidP(uint32_t nMapID, int nMapX, int nMapY)
{
    // argument check
    if(nMapID == 0 || nMapX < 0 || nMapY < 0){ return false; }

    auto pMap = m_MapRecordMap.find(nMapID);

    // we didn't try to load it yet
    if(pMap == m_MapRecordMap.end()){
        LoadMap(nMapID);
        pMap = m_MapRecordMap.find(nMapID);
    }

    // mysterious error occurs, since even nMapID is invalid, the loading will
    // put a placeholder in the map cache to prevent next time loadind
    if(pMap == m_MapRecordMap.end()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "mysterious errors when loading map");
        g_MonoServer->Restart();
    }

    // tried, but turns out it's only a place holder, not valid
    if(pMap->second.PodAddress == Theron::Address::Null()){ return false; }

    // it's a valid map, try to figure the RM parameters
    int nRMW   = pMap->second.RMW;
    int nRMH   = pMap->second.RMH;
    int nGridW = pMap->second.GridW;
    int nGridH = pMap->second.GridH;

    if(nRMW <= 0 || nRMH <= 0 || nGridW <= 0 || nGridH <= 0){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "logic error: region monitor parameter invalid");
        g_MonoServer->Restart();
    }

    int nGridX = nMapX / SYS_MAPGRIDXP;
    int nGridY = nMapY / SYS_MAPGRIDYP;

    // ooops outside
    if(nGridX >= nGridW || nGridY >= nGridH){ return false; }

    return true;
}

// query for a RM address in the local information, if local gives pending, then
// send query to corresponding map pod
//
//  nMapID      :
//  nMapX       :
//  nMapY       :
//  bAddTrigger : if false, only do local query and one possible remote query. if
//                true, after the remote query, if we get PENDING response, this
//                function will put a self-driven anynomous trigger to continue
//                to do remote query, till we get the non-pending query result
//
//                this parameter prevent we get into trouble, becase we call itself
//                if we get pending state from the remote actor. think about the
//                suitation: current trigger get pending, then it add a trigger,
//                and this trigger get pending again , then it add a trigger...
//                
// return QUERY_XXX, after this function, the nMapID always has a record, even
// it may only be a place holder, and if (nMapID, nMapX, nMapY) valid then it also
// create a RM record for it.
//
int ServiceCore::QueryRMAddress(uint32_t nMapID, int nMapX, int nMapY, bool bAddTrigger)
{
    // argument check
    if(nMapID == 0 || nMapX < 0 || nMapY < 0){ return QUERY_ERROR; }

    auto pMap = m_MapRecordMap.find(nMapID);

    // we didn't try to load it yet
    if(pMap == m_MapRecordMap.end()){
        LoadMap(nMapID);
        pMap = m_MapRecordMap.find(nMapID);
    }

    // mysterious error occurs, since even nMapID is invalid, the loading will
    // put a placeholder in the map cache to prevent next time loadind
    if(pMap == m_MapRecordMap.end()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "mysterious errors when loading map");
        g_MonoServer->Restart();
    }

    // ok now map record is valid, let's make a ref
    auto &rstMapRecord = m_MapRecordMap[nMapID];

    // tried, but turns out it's only a place holder, not valid
    if(!rstMapRecord.PodAddress){ return QUERY_ERROR; }

    // it's a valid map, try to figure the RM parameters
    int nRMW = rstMapRecord.RMW;
    int nRMH = rstMapRecord.RMH;

    if(nRMW <= 0 || nRMH <= 0){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "logic error: region monitor parameter invalid");
        g_MonoServer->Restart();
    }

    int nRMXEnd = (rstMapRecord.GridW + nRMW - 1) / nRMW;
    int nRMYEnd = (rstMapRecord.GridH + nRMH - 1) / nRMH;

    int nRMX = nMapX / SYS_MAPGRIDXP / nRMW;
    int nRMY = nMapY / SYS_MAPGRIDYP / nRMH;

    // ooops outside
    if(nRMX >= nRMXEnd || nRMY >= nRMYEnd){ return QUERY_ERROR; }

    // ok now map is ready and query parameters are all legal

    // try to find the RM record
    uint32_t nRMCacheKey = ((uint32_t)(nRMX) << 16) + ((uint32_t)nRMY);
    auto pRM = rstMapRecord.RMRecordMap.find(nRMCacheKey);

    // there is a record already, possible
    //      1. this is a record with pending state, we asked for the pending
    //      1. this is a record with pending state, we asked for the pending and it anwsers pending
    //      2. this is a checked record, but it's empty because non-walkable
    //      3. this is a well-prepared record
    //
    // we only return this record and let outside logic check the state

    // if a record exists, it should be legal always!
    if(pRM != rstMapRecord.RMRecordMap.end()){
        if((pRM->second).Valid()){ return QUERY_OK; }
        if((pRM->second).Ready()){ return QUERY_NA; } // not valid but ready, ok it's NA

        // not ready, not valid, then it should be pending
        // if not pending, ok we are in trouble
        if(!((pRM->second).MapID && (pRM->second).Query == QUERY_PENDING)){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "region monitor record error");
            g_MonoServer->Restart();
        }
    }

    // there is no such a record, or the record is in state of PENDING
    // then create a new or ref to the proper record

    auto &rstRMRecord = rstMapRecord.RMRecordMap[nRMCacheKey];

    // 1. reset it as legal pending state
    rstRMRecord.MapID      = nMapID;
    rstRMRecord.RMX        = nRMX;
    rstRMRecord.RMY        = nRMY;
    rstRMRecord.Query      = QUERY_PENDING;
    rstRMRecord.PodAddress = Theron::Address::Null();

    // 2. ask the corresponding map
    AMQueryRMAddress stAMQRMA;
    stAMQRMA.MapX  = nMapX;
    stAMQRMA.MapY  = nMapY;
    stAMQRMA.MapID = nMapID;

    // 3. define the resp handler, we need ask again if we get pending answer
    auto fnOnR = [this, nMapID, nMapX, nMapY, nRMCacheKey, bAddTrigger](const MessagePack &rstRMPK, const Theron::Address &){
        // assume (nMapID, nRMCacheKey) is always valid now
        auto &rstRMRecord = m_MapRecordMap[nMapID].RMRecordMap[nRMCacheKey];
        switch(rstRMPK.Type()){
            case MPK_ADDRESS:
                {
                    if(true
                            && rstRMPK.Data()
                            && rstRMPK.DataLen() > 0
                            && std::strlen((char *)rstRMPK.Data()) > 0){
                        rstRMRecord.Query = QUERY_OK;
                        rstRMRecord.PodAddress = Theron::Address((char *)rstRMPK.Data());
                        return;
                    }

                    // you promise it's an address but you are lying
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "response of address with null content");
                    g_MonoServer->Restart();
                    return;
                }
            case MPK_ERROR:
                {
                    // TODO: here we don't have further information
                    //       for server map if queried RM is out of boundary it won't kill process
                    //       instead it return an ERROR, and if the queried RM is invalid, it aslo
                    //       return an ERROR
                    //
                    //       so if there is any inconsitancy of current map record between, then
                    //       we couldn't handle this ERROR correctly
                    //
                    //       but if current record (cache) is good, then there could not be any
                    //       problem of ``out of boundary" since we checked it before sending this
                    //       query
                    rstRMRecord.Query = QUERY_NA;
                    rstRMRecord.PodAddress = Theron::Address::Null();
                    return;
                }
            case MPK_PENDING:
                {
                    // TODO: rule for PENDING
                    // for pending state, if A ask B and B respond with MPK_PENDING, B should
                    // finished task C to respond A with precise message, but task C is not
                    // finished till now. then B respond A with MPK_PENDING, B may (or may
                    // not) continue doing task C after this querying, however even B finished
                    // this task, it won't respond A automatically. if A need this message, A
                    // should ask again and this time since task C is done, B will respond
                    // with precise information to A
                    //
                    // this map is not ready, add a anynomous trigger, but for ServiceCore
                    // since it doesn't have a metronome, and the queried map won't respond
                    // again because of the ``rule of PENDING", then this anynomous can't
                    // be driven
                    //
                    // we have to drive it by ourself
                    
                    if(!bAddTrigger){ return; }

                    // since this is not in the message pack operation function
                    SyncDriver().Forward(MPK_DUMMY, m_ActorPod->GetAddress());

                    // since we get a pending answer, we have to ask again
                    auto fnTmpTrigger = [this, nMapID, nMapX, nMapY]() -> bool{
                        // 1. done
                        if(QueryRMAddress(nMapID, nMapX, nMapY, false) != QUERY_PENDING){ return true; }

                        // 2. otherwise we still need to drive this anyomous trigger
                        extern EventTaskHub *g_EventTaskHub;
                        g_EventTaskHub->Add(200, [stAddr = m_ActorPod->GetAddress()](){
                            SyncDriver().Forward(MPK_DUMMY, stAddr);
                        });

                        return false;
                    };

                    m_StateHook.Install(fnTmpTrigger);
                    return;
                }
            default:
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported message type %s", rstRMPK.Name());
                    g_MonoServer->Restart();
                    return;
                }
        }
    };

    m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRMA}, rstMapRecord.PodAddress, fnOnR);
    return QUERY_PENDING;
}

const ServiceCore::RMRecord &ServiceCore::GetRMRecord(uint32_t nMapID, int nMapX, int nMapY, bool bAddTrigger)
{
    // don't bother if the argument is not valid
    // this validation function will check everything of the record
    //
    if(!ValidP(nMapID, nMapX, nMapY)){ return m_EmptyRMRecord; }

    // OK the map is valid
    auto &rstMapRecord = m_MapRecordMap[nMapID];

    int nRMX = nMapX / SYS_MAPGRIDXP / rstMapRecord.RMW;
    int nRMY = nMapY / SYS_MAPGRIDYP / rstMapRecord.RMH;

    // to make (nMapID, nRMX, nRMY) valid in the cache
    // add a trigger for the RM address
    QueryRMAddress(nMapID, nMapX, nMapY, bAddTrigger);

    return m_MapRecordMap[nMapID].RMRecordMap[((uint32_t)(nRMX) << 16) + ((uint32_t)nRMY)];
}
