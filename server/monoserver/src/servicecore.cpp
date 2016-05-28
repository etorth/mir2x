/*
 * =====================================================================================
 *
 *       Filename: servicecore.cpp
 *        Created: 04/22/2016 18:16:53
 *  Last Modified: 05/27/2016 22:27:04
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
        // case MPK_LOGIN:
        //     {
        //         On_MPK_LOGIN(rstMPK, rstAddr);
        //         break;
        //     }
        case MPK_PLAYERPHATOM:
            {
                On_MPK_PLAYERPHATOM(rstMPK, rstAddr);
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING,
                        "unsupported message type: (%d:%s)", rstMPK.Type(), rstMPK.Name());
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
    int nGridY = nMapX / SYS_MAPGRIDYP;

    // ooops outside
    if(nGridX >= nGridW || nGridY >= nGridH){ return false; }

    return true;
}

const ServiceCore::RMRecord &ServiceCore::GetRMRecord(uint32_t nMapID, int nMapX, int nMapY)
{
    // don't bother if the argument is not valid
    // this validation function will check everything of the record
    //
    if(!ValidP(nMapID, nMapX, nMapY)){ return m_EmptyRMRecord; }

    // OK valid
    auto &rstMapRecord = m_MapRecordMap[nMapID];

    int nRMX = nMapX / SYS_MAPGRIDXP / rstMapRecord.RMW;
    int nRMY = nMapX / SYS_MAPGRIDYP / rstMapRecord.RMH;

    // the map is valid and the coord is ok
    uint32_t nRMCacheKey = ((uint32_t)(nRMX) << 16) + ((uint32_t)nRMY);
    auto pRM = rstMapRecord.RMRecordMap.find(nRMCacheKey);

    // there is a record already, possible
    //      1. this is a record with pending state
    //      2. this is a checked record, but it's empty because non-walkable
    //      3. this is a well-prepared record
    //
    // we only return this record and let outside logic check the state
    //
    if(pRM != rstMapRecord.RMRecordMap.end()){ return pRM->second; }

    // current the nMapID, nRMX, nRMY all are legal, but we haven't acquire the RM yet

    // create a record firstly
    auto &rstRMRecord = rstMapRecord.RMRecordMap[nRMCacheKey];

    rstRMRecord.MapID      = nMapID;
    rstRMRecord.RMX        = nRMX;
    rstRMRecord.RMY        = nRMY;
    rstRMRecord.Query      = QUERY_PENDING;
    rstRMRecord.PodAddress = Theron::Address::Null();

    // we need to ask from the map
    AMQueryRMAddress stAMQRMA;
    stAMQRMA.RMX   = nRMX;
    stAMQRMA.RMY   = nRMY;
    stAMQRMA.MapID = nMapID;

    auto fnDone = [this, nMapID, nRMCacheKey](const MessagePack &rstRMPK, const Theron::Address &){
        auto &rstRMRecord = m_MapRecordMap[nMapID].RMRecordMap[nRMCacheKey];

        // make it very strict
        if(true
                && rstRMPK.Type() == MPK_ADDRESS
                && rstRMPK.Data() != nullptr
                && rstRMPK.DataLen() > 0
                && std::strlen((char *)rstRMPK.Data()) > 0){
            rstRMRecord.Query = QUERY_OK;
            rstRMRecord.PodAddress = Theron::Address((char *)rstRMPK.Data());
        }else{
            rstRMRecord.Query = QUERY_NA;
            rstRMRecord.PodAddress = Theron::Address::Null();
        }
    };

    m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRMA}, rstMapRecord.PodAddress, fnDone);
    return rstRMRecord;
}
