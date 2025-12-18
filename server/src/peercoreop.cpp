#include "sgf.hpp"
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
#include "peerconfig.hpp"

extern Server *g_server;
extern PeerConfig *g_peerConfig;

corof::awaitable<> PeerCore::on_AM_PEERCONFIG(const ActorMsgPack &mpk)
{
    g_peerConfig->setConfig(mpk.deserialize<SDPeerConfig>());
    return {};
}

corof::awaitable<> PeerCore::on_AM_PEERLOADMAP(const ActorMsgPack &mpk)
{
    // map may run on peer
    // but is manageed on service core

    auto loadMapSg = sgf::guard([fromAddr = mpk.fromAddr(), this]()
    {
        m_actorPod->post(fromAddr, AM_ERROR);
    });

    const auto amPLM = mpk.conv<AMPeerLoadMap>();

    if(!uidsf::isLocalUID(amPLM.mapUID)){
        co_return;
    }

    const auto [loaded, newLoad] = loadMap(amPLM.mapUID);
    if(!loaded){
        co_return;
    }

    if(amPLM.waitActivated){
        if(const auto loadMpk = co_await m_actorPod->send(amPLM.mapUID, AM_WAITACTIVATED); loadMpk.type() != AM_WAITACTIVATEDOK){
            co_return;
        }
    }

    AMPeerLoadMapOK amPLMOK;
    std::memset(&amPLMOK, 0, sizeof(amPLMOK));

    amPLMOK.newLoad = newLoad;
    m_actorPod->post(mpk.fromAddr(), {AM_PEERLOADMAPOK, amPLMOK});
    loadMapSg.dismiss();

    if(newLoad){
        g_server->addLog(LOGTYPE_INFO, "Load map %d on peer %zu successfully", to_d(uidf::getMapID(amPLM.mapUID)), uidf::peerIndex(UID()));
    }
}
