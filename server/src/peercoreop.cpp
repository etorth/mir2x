#include "uidf.hpp"
#include "uidsf.hpp"
#include "serverguard.hpp"
#include "player.hpp"
#include "npchar.hpp"
#include "monster.hpp"
#include "peercore.hpp"
#include "servicecore.hpp"
#include "serdesmsg.hpp"
#include "server.hpp"

extern Server *g_server;
void PeerCore::on_AM_PEERLOADMAP(const ActorMsgPack &mpk)
{
    // map may run on peer
    // but is manageed on service core

    const auto amPLM = mpk.conv<AMPeerLoadMap>();

    if(!uidsf::isLocalUID(amPLM.mapUID)){
        m_actorPod->forward(mpk.fromAddr(), AM_ERROR);
        return;
    }

    if(auto [loaded, newLoad] = loadMap(amPLM.mapUID); loaded){
        AMPeerLoadMapOK amPLMOK;
        std::memset(&amPLMOK, 0, sizeof(amPLMOK));

        amPLMOK.newLoad = newLoad;
        m_actorPod->forward(mpk.fromAddr(), {AM_PEERLOADMAPOK, amPLMOK});
        if(newLoad){
            g_server->addLog(LOGTYPE_INFO, "Load map %d on peer %zu successfully.", to_d(uidf::getMapID(amPLM.mapUID)), uidf::peerIndex(UID()));
        }
    }
    else{
        m_actorPod->forward(mpk.fromAddr(), AM_ERROR);
    }
}
