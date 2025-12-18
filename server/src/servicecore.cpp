#include <string>
#include <cstring>
#include "uidf.hpp"
#include "uidsf.hpp"
#include "quest.hpp"
#include "player.hpp"
#include "totype.hpp"
#include "filesys.hpp"
#include "actorpod.hpp"
#include "mapbindb.hpp"
#include "serdesmsg.hpp"
#include "server.hpp"
#include "servicecore.hpp"
#include "serverargparser.hpp"
#include "serverconfigurewindow.hpp"

extern MapBinDB *g_mapBinDB;
extern Server *g_server;
extern ServerArgParser *g_serverArgParser;
extern ServerConfigureWindow *g_serverConfigureWindow;

ServiceCore::ServiceCore()
    : PeerCore()
{}

corof::awaitable<> ServiceCore::onActorMsg(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_BADCHANNEL:
            {
                return on_AM_BADCHANNEL(mpk);
            }
        case AM_REGISTERQUEST:
            {
                return on_AM_REGISTERQUEST(mpk);
            }
        case AM_RECVPACKAGE:
            {
                return on_AM_RECVPACKAGE(mpk);
            }
        case AM_QUERYMAPLIST:
            {
                return on_AM_QUERYMAPLIST(mpk);
            }
        case AM_QUERYCOCOUNT:
            {
                return on_AM_QUERYCOCOUNT(mpk);
            }
        case AM_LOADMAP:
            {
                return on_AM_LOADMAP(mpk);
            }
        case AM_MODIFYQUESTTRIGGERTYPE:
            {
                return on_AM_MODIFYQUESTTRIGGERTYPE(mpk);
            }
        case AM_QUERYQUESTTRIGGERLIST:
            {
                return on_AM_QUERYQUESTTRIGGERLIST(mpk);
            }
        case AM_QUERYQUESTUID:
            {
                return on_AM_QUERYQUESTUID(mpk);
            }
        case AM_QUERYQUESTUIDLIST:
            {
                return on_AM_QUERYQUESTUIDLIST(mpk);
            }
        default:
            {
                throw fflvalue(mpk.str());
            }
    }
}

corof::awaitable<> ServiceCore::operateNet(uint32_t channID, uint8_t cmType, const uint8_t *buf, size_t bufSize, uint64_t respID)
{
    switch(cmType){
        case CM_LOGIN:
            {
                return net_CM_LOGIN(channID, cmType, buf, bufSize, respID);
            }
        case CM_ONLINE:
            {
                return net_CM_ONLINE(channID, cmType, buf, bufSize, respID);
            }
        case CM_QUERYCHAR:
            {
                return net_CM_QUERYCHAR(channID, cmType, buf, bufSize, respID);
            }
        case CM_CREATECHAR:
            {
                return net_CM_CREATECHAR(channID, cmType, buf, bufSize, respID);
            }
        case CM_DELETECHAR:
            {
                return net_CM_DELETECHAR(channID, cmType, buf, bufSize, respID);
            }
        case CM_CREATEACCOUNT:
            {
                return net_CM_CREATEACCOUNT(channID, cmType, buf, bufSize, respID);
            }
        case CM_CHANGEPASSWORD:
            {
                return net_CM_CHANGEPASSWORD(channID, cmType, buf, bufSize, respID);
            }
        default:
            {
                throw fflvalue(ClientMsg(cmType).name());
            }
    }
}

corof::awaitable<> ServiceCore::onActivate()
{
    co_await ServerObject::onActivate();
    m_addCO = std::make_unique<EnableAddCO>(m_actorPod);

    std::set<uint64_t> loadedMapList;
    for(uint32_t mapID = 1; mapID < DBCOM_MAPENDID(); ++mapID){
        if(g_serverArgParser->masterConfig().preloadMapCheck(mapID)){
            const uint64_t mapUID = uidsf::getMapBaseUID(mapID);
            if(const auto [loaded, _] = co_await requestLoadMap(mapUID, false); loaded){
                loadedMapList.insert(mapUID);
                g_server->addLog(LOGTYPE_INFO, "Preload %s successfully", to_cstr(DBCOM_MAPRECORD(mapID).name));
            }
            else{
                throw fflerror("failed to load map %s", to_cstr(DBCOM_MAPRECORD(mapID).name));
            }
        }
    }

    for(const auto mapUID: loadedMapList){
        switch(const auto mpk = co_await m_actorPod->send(mapUID, AM_WAITACTIVATED); mpk.type()){
            case AM_WAITACTIVATEDOK:
                {
                    break;
                }
            default:
                {
                    throw fflerror("activation failed: %s", to_cstr(uidf::getUIDString(mapUID)));
                }
        }
    }

    if(!g_serverArgParser->masterConfig().disableQuestScript){
        const auto cfgScriptPath = g_serverConfigureWindow->getConfig().scriptPath;
        const auto scriptPath = cfgScriptPath.empty() ? std::string("script/quest") : (cfgScriptPath + "/quest");
        const auto questFileNameRegex = []() -> std::string
        {
            if(g_serverArgParser->masterConfig().loadSingleQuest.empty()){
                return R"#(.*\.lua)#";
            }
            else{
                return g_serverArgParser->masterConfig().loadSingleQuest + R"#(\.lua)#";
            }
        }();

        for(uint32_t questID = 1; const auto &fileName: filesys::getFileList(scriptPath.c_str(), false, questFileNameRegex.c_str())){
            if(auto questPtr = new Quest(SDInitQuest
            {
                .questID = questID++,
                .fullScriptName = scriptPath + "/" + fileName,
            })){
                questPtr->activate();
            }
        }
    }
}

corof::awaitable<std::pair<bool, bool>> ServiceCore::requestLoadMap(uint64_t mapUID, bool waitActivated)
{
    std::pair<bool, bool> result
    {
        false,  //  loaded
        false,  // newLoad
    };

#define result_WAIT_MAP_ACTIVATED() \
    if(result.first && waitActivated){ \
        if(const auto mpk = co_await m_actorPod->send(mapUID, AM_WAITACTIVATED); mpk.type() != AM_WAITACTIVATEDOK){ \
            result.first = false; \
        } \
    } \

    if(m_mapList.contains(mapUID)){
        result.first  = true;
        result.second = false;

        result_WAIT_MAP_ACTIVATED();
        co_return result;
    }

    if(uidsf::isLocalUID(mapUID)){
        result = loadMap(mapUID);
        result_WAIT_MAP_ACTIVATED();
        co_return result;
    }

    if(auto p = m_loadMapPendingOps.find(mapUID); p != m_loadMapPendingOps.end()){
        result = co_await RegisterLoadMapOpAwaiter
        {
            .core = this,
            .mapUID = mapUID,
        };

        result_WAIT_MAP_ACTIVATED();
        co_return result;
    }

#undef result_WAIT_MAP_ACTIVATED

    AMPeerLoadMap amPLM;
    std::memset(&amPLM, 0, sizeof(amPLM));

    amPLM.mapUID = mapUID;
    amPLM.waitActivated = waitActivated;

    m_loadMapPendingOps.try_emplace(mapUID);

    const auto mpk = co_await m_actorPod->send(uidf::getPeerCoreUID(uidf::peerIndex(mapUID)), {AM_PEERLOADMAP, amPLM});
    const bool loaded = (mpk.type() == AM_PEERLOADMAPOK);

    if(loaded){
        m_mapList.insert(mapUID);
    }

    for(auto &h: m_loadMapPendingOps.at(mapUID)){
        h.resume();
    }

    m_loadMapPendingOps.erase(mapUID); // won't keep record of bad load

    result.first  = loaded;
    result.second = loaded ? true : false;

    co_return result;
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
