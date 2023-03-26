#include <string>
#include <cstring>
#include "player.hpp"
#include "totype.hpp"
#include "actorpod.hpp"
#include "mapbindb.hpp"
#include "monoserver.hpp"
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
        case AM_ADDCO:
            {
                on_AM_ADDCO(rstMPK);
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
        case AM_LOADMAP:
            {
                on_AM_LOADMAP(rstMPK);
                break;
            }
        default:
            {
                g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported message: %s", mpkName(rstMPK.type()));
                break;
            }
    }
}

void ServiceCore::operateNet(uint32_t channID, uint8_t cmType, const uint8_t *buf, size_t bufSize)
{
    switch(cmType){
        case CM_LOGIN:
            {
                net_CM_LOGIN(channID, cmType, buf, bufSize);
                break;
            }
        case CM_ONLINE:
            {
                net_CM_ONLINE(channID, cmType, buf, bufSize);
                break;
            }
        case CM_QUERYCHAR:
            {
                net_CM_QUERYCHAR(channID, cmType, buf, bufSize);
                break;
            }
        case CM_CREATECHAR:
            {
                net_CM_CREATECHAR(channID, cmType, buf, bufSize);
                break;
            }
        case CM_DELETECHAR:
            {
                net_CM_DELETECHAR(channID, cmType, buf, bufSize);
                break;
            }
        case CM_CREATEACCOUNT:
            {
                net_CM_CREATEACCOUNT(channID, cmType, buf, bufSize);
                break;
            }
        case CM_CHANGEPASSWORD:
            {
                net_CM_CHANGEPASSWORD(channID, cmType, buf, bufSize);
                break;
            }
        default:
            {
                throw fflerror("unknown client message unhandled: %s", to_cstr(ClientMsg(cmType).name()));
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

    m_mapList.insert_or_assign(mapID, new ServerMap(mapID)).first->second->activate();
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

std::optional<std::pair<uint32_t, bool>> ServiceCore::findDBID(uint32_t channID) const
{
    fflassert(channID);
    if(auto p = m_dbidList.find(channID); p != m_dbidList.end()){
        fflassert(p->second.first);
        return p->second;
    }
    return {};
}
