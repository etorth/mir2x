/*
 * =====================================================================================
 *
 *       Filename: servicecore.cpp
 *        Created: 04/22/2016 18:16:53
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
#include "mapbindb.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"

extern MapBinDB *g_MapBinDB;

ServiceCore::ServiceCore()
    : ServerObject(UIDFunc::GetServiceCoreUID())
    , m_MapList()
{}

void ServiceCore::OperateAM(const MessagePack &rstMPK)
{
    switch(rstMPK.Type()){
        case MPK_BADCHANNEL:
            {
                On_MPK_BADCHANNEL(rstMPK);
                break;
            }
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK);
                break;
            }
        case MPK_ADDCHAROBJECT:
            {
                On_MPK_ADDCHAROBJECT(rstMPK);
                break;
            }
        case MPK_TRYMAPSWITCH:
            {
                On_MPK_TRYMAPSWITCH(rstMPK);
                break;
            }
        case MPK_NETPACKAGE:
            {
                On_MPK_NETPACKAGE(rstMPK);
                break;
            }
        case MPK_QUERYMAPLIST:
            {
                On_MPK_QUERYMAPLIST(rstMPK);
                break;
            }
        case MPK_QUERYCOCOUNT:
            {
                On_MPK_QUERYCOCOUNT(rstMPK);
                break;
            }
        case MPK_QUERYMAPUID:
            {
                On_MPK_QUERYMAPUID(rstMPK);
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Unsupported message: %s", rstMPK.Name());
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
    if(nMapID){
        if(m_MapList.find(nMapID) == m_MapList.end()){
            if(g_MapBinDB->Retrieve(nMapID)){
                auto pMap = new ServerMap(this, nMapID);
                pMap->Activate();
                m_MapList[nMapID] = pMap;
                return true;
            }else{
                return false;
            }
        }
        return true;
    }
    return false;
}

const ServerMap *ServiceCore::RetrieveMap(uint32_t nMapID)
{
    if(nMapID){
        auto pMap = m_MapList.find(nMapID);
        if(pMap == m_MapList.end()){
            if(LoadMap(nMapID)){
                pMap = m_MapList.find(nMapID);
            }
        }

        return (pMap == m_MapList.end()) ? nullptr : pMap->second;
    }
    return nullptr;
}
