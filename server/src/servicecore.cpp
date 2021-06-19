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

#include <string>
#include <cstring>
#include "player.hpp"
#include "totype.hpp"
#include "actorpod.hpp"
#include "mapbindb.hpp"
#include "monoserver.hpp"
#include "dbcomrecord.hpp"
#include "servicecore.hpp"
#include "serverargparser.hpp"

extern MapBinDB *g_mapBinDB;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

ServiceCore::ServiceCore()
    : ServerObject(uidf::getServiceCoreUID())
    , m_mapList()
{}

void ServiceCore::operateAM(const ActorMsgPack &rstMPK)
{
    switch(rstMPK.type()){
        case AM_BADCHANNEL:
            {
                on_AM_BADCHANNEL(rstMPK);
                break;
            }
        case AM_METRONOME:
            {
                on_AM_METRONOME(rstMPK);
                break;
            }
        case AM_ADDCHAROBJECT:
            {
                on_AM_ADDCHAROBJECT(rstMPK);
                break;
            }
        case AM_RECVPACKAGE:
            {
                on_AM_RECVPACKAGE(rstMPK);
                break;
            }
        case AM_QUERYMAPLIST:
            {
                on_AM_QUERYMAPLIST(rstMPK);
                break;
            }
        case AM_QUERYCOCOUNT:
            {
                on_AM_QUERYCOCOUNT(rstMPK);
                break;
            }
        case AM_QUERYMAPUID:
            {
                on_AM_QUERYMAPUID(rstMPK);
                break;
            }
        default:
            {
                g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported message: %s", mpkName(rstMPK.type()));
                break;
            }
    }
}

void ServiceCore::operateNet(uint32_t nSID, uint8_t nType, const uint8_t *pData, size_t nDataLen)
{
    switch(nType){
        case CM_LOGIN:
            {
                net_CM_Login(nSID, nType, pData, nDataLen);
                break;
            }
        case CM_ACCOUNT:
            {
                net_CM_Account(nSID, nType, pData, nDataLen);
                break;
            }
        default:
            {
                break;
            }
    }
}

void ServiceCore::onActivate()
{
    ServerObject::onActivate();
    if(const auto mapID = g_serverArgParser->preloadMapID; mapID > 0){
        if(retrieveMap(mapID)){
            g_monoServer->addLog(LOGTYPE_INFO, "Preload %s successfully", to_cstr(DBCOM_MAPRECORD(mapID).name));
        }
    }

    if(g_serverArgParser->preloadMap){
        for(uint32_t mapID = 1;; ++mapID){
            if(!retrieveMap(mapID)){
                break;
            }
            g_monoServer->addLog(LOGTYPE_INFO, "Preload %s successfully", to_cstr(DBCOM_MAPRECORD(mapID).name));
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

    if(!g_mapBinDB->retrieve(mapID)){
        return;
    }

    auto mapPtr = new ServerMap(mapID);
    mapPtr->activate();
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
