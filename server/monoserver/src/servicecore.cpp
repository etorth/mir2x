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
    : ServerObject(uidf::buildServiceCoreUID())
    , m_mapList()
{}

void ServiceCore::operateAM(const MessagePack &rstMPK)
{
    switch(rstMPK.Type()){
        case MPK_BADCHANNEL:
            {
                on_MPK_BADCHANNEL(rstMPK);
                break;
            }
        case MPK_METRONOME:
            {
                on_MPK_METRONOME(rstMPK);
                break;
            }
        case MPK_ADDCHAROBJECT:
            {
                on_MPK_ADDCHAROBJECT(rstMPK);
                break;
            }
        case MPK_NETPACKAGE:
            {
                on_MPK_NETPACKAGE(rstMPK);
                break;
            }
        case MPK_QUERYMAPLIST:
            {
                on_MPK_QUERYMAPLIST(rstMPK);
                break;
            }
        case MPK_QUERYCOCOUNT:
            {
                on_MPK_QUERYCOCOUNT(rstMPK);
                break;
            }
        case MPK_QUERYMAPUID:
            {
                on_MPK_QUERYMAPUID(rstMPK);
                break;
            }
        default:
            {
                g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported message: %s", rstMPK.Name());
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
        default:
            {
                break;
            }
    }
}

uint64_t ServiceCore::activate()
{
    if(const auto uid = ServerObject::activate()){
        if(g_serverArgParser->preloadMap){
            for(uint32_t mapID = 1;; ++mapID){
                if(!retrieveMap(mapID)){
                    return uid;
                }
                g_monoServer->addLog(LOGTYPE_INFO, "Preload %s successfully", to_cstr(DBCOM_MAPRECORD(mapID).name));
            }
        }
    }
    return 0;
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
