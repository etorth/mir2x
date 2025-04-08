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
#include "monoserver.hpp"
#include "servicecore.hpp"
#include "serverargparser.hpp"
#include "serverconfigurewindow.hpp"

extern MapBinDB *g_mapBinDB;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;
extern ServerConfigureWindow *g_serverConfigureWindow;

ServiceCore::ServiceCore()
    : ServerObject(uidsf::getServiceCoreUID())
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
        case AM_REGISTERQUEST:
            {
                on_AM_REGISTERQUEST(rstMPK);
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
        case AM_MODIFYQUESTTRIGGERTYPE:
            {
                on_AM_MODIFYQUESTTRIGGERTYPE(rstMPK);
                break;
            }
        case AM_QUERYQUESTTRIGGERLIST:
            {
                on_AM_QUERYQUESTTRIGGERLIST(rstMPK);
                break;
            }
        case AM_QUERYQUESTUID:
            {
                on_AM_QUERYQUESTUID(rstMPK);
                break;
            }
        case AM_QUERYQUESTUIDLIST:
            {
                on_AM_QUERYQUESTUIDLIST(rstMPK);
                break;
            }
        default:
            {
                g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported message: %s", mpkName(rstMPK.type()));
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
    if(const auto mapID = g_serverArgParser->preloadMapID; mapID > 0){
        loadMap(mapID, [mapID](bool)
        {
            g_monoServer->addLog(LOGTYPE_INFO, "Preload %s successfully", to_cstr(DBCOM_MAPRECORD(mapID).name));
        });
    }

    if(g_serverArgParser->preloadMap){
        for(uint32_t mapID = 1; mapID < DBCOM_MAPENDID(); ++mapID){
            loadMap(mapID, [mapID](bool)
            {
                g_monoServer->addLog(LOGTYPE_INFO, "Preload %s successfully", to_cstr(DBCOM_MAPRECORD(mapID).name));
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

void ServiceCore::loadMap(uint32_t mapID, std::function<void(bool)> onDone, std::function<void()> onError)
{
    if(!mapID){
        if(onError){
            onError();
        }
        return;
    }

    if(!g_mapBinDB->retrieve(mapID)){
        if(onError){
            onError();
        }
        return;
    }

    if(auto p = m_loadMapOps.find(mapID); p != m_loadMapOps.end()){
        if(p->second.pending){
            if(onDone){
                p->second.onDone.push_back(std::move(onDone));
            }

            if(onError){
                p->second.onError.push_back(std::move(onError));
            }
        }
        else{
            if(onDone){
                onDone(false);
            }
        }
        return;
    }

    if(const auto mapUID = uidf::getMapBaseUID(mapID); uidsf::isLocalUID(mapUID)){
        auto mapPtr = new ServerMap(mapID);
        mapPtr->activate();

        m_loadMapOps[mapID].pending = false;
        if(onDone){
            onDone(true);
        }
    }
    else{
        AMAddMap amAM;
        std::memset(&amAM, 0, sizeof(amAM));
        amAM.mapID = mapID;

        m_loadMapOps[mapID].pending = true;
        m_actorPod->forward(uidf::getServiceCoreUID(uidsf::peerIndex(mapUID)), {AM_ADDMAP, amAM}, [mapID, onDone = std::move(onDone), onError = std::move(onError), this](const ActorMsgPack &mpk)
        {
            switch(mpk.type()){
                case AM_ADDMAPOK:
                    {
                        for(auto &onDone: m_loadMapOps.at(mapID).onDone){
                            if(onDone){
                                onDone(true);
                            }
                        }
                        break;
                    }
                default:
                    {
                        for(auto &onError: m_loadMapOps.at(mapID).onError){
                            if(onError){
                                onError();
                            }
                        }
                        break;
                    }
            }
        });
    }
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
