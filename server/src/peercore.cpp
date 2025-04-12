#include <string>
#include <cstring>
#include "uidf.hpp"
#include "uidsf.hpp"
#include "quest.hpp"
#include "player.hpp"
#include "totype.hpp"
#include "filesys.hpp"
#include "rotatecoord.hpp"
#include "actorpod.hpp"
#include "mapbindb.hpp"
#include "serdesmsg.hpp"
#include "server.hpp"
#include "servicecore.hpp"
#include "npchar.hpp"
#include "serverargparser.hpp"
#include "serverconfigurewindow.hpp"

extern MapBinDB *g_mapBinDB;
extern Server *g_server;
extern ServerArgParser *g_serverArgParser;
extern ServerConfigureWindow *g_serverConfigureWindow;

PeerCore::PeerCore()
    : ServerObject(uidsf::getPeerCoreUID())
{}

std::pair<bool, bool> PeerCore::loadMap(uint64_t mapUID)
{
    if(!g_mapBinDB->retrieve(uidf::getMapID(mapUID))){
        return {false, false};
    }

    if(m_mapList.contains(mapUID)){
        return {true, false};
    }

    auto mapPtr = new ServerMap(mapUID);
    mapPtr->activate();

    m_mapList.insert(mapUID);
    return {true, true};
}

void PeerCore::operateAM(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_PEERLOADMAP:
            {
                on_AM_PEERLOADMAP(mpk);
                break;
            }
        default:
            {
                g_server->addLog(LOGTYPE_WARNING, "Unsupported message: %s", mpkName(mpk.type()));
                break;
            }
    }
}
