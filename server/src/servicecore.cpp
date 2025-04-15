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

void ServiceCore::operateAM(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_BADCHANNEL:
            {
                on_AM_BADCHANNEL(mpk);
                break;
            }
        case AM_METRONOME:
            {
                on_AM_METRONOME(mpk);
                break;
            }
        case AM_REGISTERQUEST:
            {
                on_AM_REGISTERQUEST(mpk);
                break;
            }
        case AM_RECVPACKAGE:
            {
                on_AM_RECVPACKAGE(mpk);
                break;
            }
        case AM_QUERYMAPLIST:
            {
                on_AM_QUERYMAPLIST(mpk);
                break;
            }
        case AM_QUERYCOCOUNT:
            {
                on_AM_QUERYCOCOUNT(mpk);
                break;
            }
        case AM_LOADMAP:
            {
                on_AM_LOADMAP(mpk);
                break;
            }
        case AM_MODIFYQUESTTRIGGERTYPE:
            {
                on_AM_MODIFYQUESTTRIGGERTYPE(mpk);
                break;
            }
        case AM_QUERYQUESTTRIGGERLIST:
            {
                on_AM_QUERYQUESTTRIGGERLIST(mpk);
                break;
            }
        case AM_QUERYQUESTUID:
            {
                on_AM_QUERYQUESTUID(mpk);
                break;
            }
        case AM_QUERYQUESTUIDLIST:
            {
                on_AM_QUERYQUESTUIDLIST(mpk);
                break;
            }
        default:
            {
                g_server->addLog(LOGTYPE_WARNING, "Unsupported message: %s", mpkName(mpk.type()));
                break;
            }
    }
}

void ServiceCore::operateNet(uint32_t channID, uint8_t cmType, const uint8_t *buf, size_t bufSize, uint64_t respID)
{
    switch(cmType){
        case CM_LOGIN:
            {
                net_CM_LOGIN(channID, cmType, buf, bufSize, respID);
                break;
            }
        case CM_ONLINE:
            {
                net_CM_ONLINE(channID, cmType, buf, bufSize, respID);
                break;
            }
        case CM_QUERYCHAR:
            {
                net_CM_QUERYCHAR(channID, cmType, buf, bufSize, respID);
                break;
            }
        case CM_CREATECHAR:
            {
                net_CM_CREATECHAR(channID, cmType, buf, bufSize, respID);
                break;
            }
        case CM_DELETECHAR:
            {
                net_CM_DELETECHAR(channID, cmType, buf, bufSize, respID);
                break;
            }
        case CM_CREATEACCOUNT:
            {
                net_CM_CREATEACCOUNT(channID, cmType, buf, bufSize, respID);
                break;
            }
        case CM_CHANGEPASSWORD:
            {
                net_CM_CHANGEPASSWORD(channID, cmType, buf, bufSize, respID);
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
    m_addCO = std::make_unique<EnableAddCO>(m_actorPod);

    for(uint32_t mapID = 1;; ++mapID){
        if(g_serverArgParser->preloadMapCheck(mapID)){
            requestLoadMap(uidsf::getMapBaseUID(mapID), [mapID](bool)
            {
                g_server->addLog(LOGTYPE_INFO, "Preload %s successfully", to_cstr(DBCOM_MAPRECORD(mapID).name));
            });
        }
    }

    if(!g_serverArgParser->disableQuestScript){
        const auto cfgScriptPath = g_serverConfigureWindow->getConfig().scriptPath;
        const auto scriptPath = cfgScriptPath.empty() ? std::string("script/quest") : (cfgScriptPath + "/quest");
        const auto questFileNameRegex = []() -> std::string
        {
            if(g_serverArgParser->loadSingleQuest.empty()){
                return R"#(.*\.lua)#";
            }
            else{
                return g_serverArgParser->loadSingleQuest + R"#(\.lua)#";
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

void ServiceCore::requestLoadMap(uint64_t mapUID, std::function<void(bool)> onDone, std::function<void()> onError)
{
    if(uidsf::isLocalUID(mapUID)){
        if(const auto [loaded, newLoad] = loadMap(mapUID); loaded){
            if(onDone){
                onDone(newLoad);
            }
        }
        else{
            if(onError){
                onError();
            }
        }
        return;
    }

    if(auto p = m_loadMapPendingOps.find(mapUID); p != m_loadMapPendingOps.end()){
        if(onDone){
            p->second.onDone.push_back(std::move(onDone));
        }

        if(onError){
            p->second.onError.push_back(std::move(onError));
        }
        return;
    }

    AMPeerLoadMap amPLM;
    std::memset(&amPLM, 0, sizeof(amPLM));
    amPLM.mapUID = mapUID;

    auto &ops = m_loadMapPendingOps.insert_or_assign(mapUID, LoadMapOp{}).first->second;

    ops.onDone .push_back(std::move(onDone));
    ops.onError.push_back(std::move(onError));

    m_actorPod->forward(uidf::getPeerCoreUID(uidf::peerIndex(mapUID)), {AM_PEERLOADMAP, amPLM}, [mapUID, this](const ActorMsgPack &mpk)
    {
        switch(mpk.type()){
            case AM_PEERLOADMAPOK:
                {
                    for(auto &onDone: m_loadMapPendingOps.at(mapUID).onDone){
                        if(onDone){
                            onDone(true);
                        }
                    }

                    m_mapList.insert(mapUID);
                    m_loadMapPendingOps.erase(mapUID);
                    break;
                }
            default:
                {
                    for(auto &onError: m_loadMapPendingOps.at(mapUID).onError){
                        if(onError){
                            onError();
                        }
                    }

                    m_loadMapPendingOps.erase(mapUID); // won't keep record of bad load
                    break;
                }
        }
    });
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
