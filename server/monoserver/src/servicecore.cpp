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

extern MapBinDB *g_mapBinDB;

ServiceCore::ServiceCore()
    : ServerObject(uidf::buildServiceCoreUID())
    , m_mapList()
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
                extern MonoServer *g_monoServer;
                g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported message: %s", rstMPK.Name());
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

void ServiceCore::loadMap(uint32_t mapID)
{
    if(!mapID){
        return;
    }

    if(m_mapList.count(mapID)){
        return;
    }

    if(!g_mapBinDB->Retrieve(mapID)){
        return;
    }

    auto mapPtr = new ServerMap(this, mapID);
    mapPtr->Activate();
    m_mapList[mapID] = mapPtr;
}

const ServerMap *ServiceCore::retrieveMap(uint32_t mapID)
{
    if(!mapID){
        return nullptr;
    }

    if(!m_mapList.count(mapID)){
        loadMap(mapID);
    }

    if(auto p = m_mapList.find(mapID); p != m_mapList.end()){
        return p->second;
    }
    return nullptr;
}
