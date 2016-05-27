/*
 * =====================================================================================
 *
 *       Filename: servicecore.cpp
 *        Created: 04/22/2016 18:16:53
 *  Last Modified: 05/26/2016 18:43:48
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

#include <system_error>

#include "player.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"

static int s_Count = 0;

ServiceCore::ServiceCore()
    : Transponder()
{
    s_Count++;
    if(s_Count > 1){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "one service core please");
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
        // case MPK_ADDMONSTER:
        //     {
        //         On_MPK_ADDMONSTER(rstMPK, rstAddr);
        //         break;
        //     }
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

    ServerMap *pNewMap = new ServerMap(nMapID);

    m_MapRecordMap[nMapID].MapID      = nMapID;
    m_MapRecordMap[nMapID].Map        = pNewMap;
    m_MapRecordMap[nMapID].PodAddress = pNewMap->Activate();

    m_ActorPod->Forward(MPK_HI, m_MapRecordMap[nMapID].PodAddress);

    return true;
}

Theron::Address ServiceCore::GetRMAddress(uint32_t nMapID, int nRMX, int nRMY)
{
    // parameter check
    if(!nMapID || nRMX < 0 || nRMY < 0){ return Theron::Address::Null(); }

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
    if(pMap->second.PodAddress == Theron::Address::Null()){
        return Theron::Address::Null();
    }

    // this RM is not valid for current map
    if((size_t)nRMX >= pMap->second.RMW || (size_t)nRMY >= pMap->second.RMH){
        return Theron::Address::Null();
    }

    // the map is valid and the coord is ok
    uint64_t nRMCacheKey = ((uint32_t)(nRMX) << 16) + ((uint32_t)nRMY);
    auto pRM = pMap->second.RMRecordMap.find(nRMCacheKey);

    // there is a record already, maybe still null but at least the addres is OTW
    if(pRM != pMap->second.RMRecordMap.end()){
        return pRM->second.PodAddress;
    }

    // current the nMapID, nRMX, nRMY all are legal, so capture it directly
    auto &rstRMRecord = (pMap->second.RMRecordMap)[nRMCacheKey];

    rstRMRecord.RMX        = nRMX;
    rstRMRecord.RMY        = nRMY;
    rstRMRecord.MapID      = nMapID;
    rstRMRecord.PodAddress = Theron::Address::Null();

    // we need to ask from the map
    AMQueryRMAddress stAMQRMA;
    stAMQRMA.RMX   = nRMX;
    stAMQRMA.RMY   = nRMY;
    stAMQRMA.MapID = nMapID;

    auto fnDoneQuery = [this, nMapID, nRMCacheKey](
            const MessagePack &rstMPK, const Theron::Address &){
        Theron::Address rstRMAddr((char *)rstMPK.Data());
        m_MapRecordMap[nMapID].RMRecordMap[nRMCacheKey].PodAddress = rstRMAddr;
    };

    m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRMA}, pMap->second.PodAddress, fnDoneQuery);
    return Theron::Address::Null();
}
