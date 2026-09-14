#include "stdf.hpp"
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

corof::awaitable<> PeerCore::on_AM_LOADMAP(const ActorMsgPack &mpk)
{
    // map may run on peer
    // but is manageed on service core

    const auto amLM = mpk.conv<AMLoadMap>();

    fflassert(uidf::isServiceCore(mpk.from()));
    fflassert(uidf::isMap(amLM.mapUID));
    fflassert(uidsf::isLocalUID(amLM.mapUID));

    const auto [loaded, newLoad] = loadMap(amLM.mapUID);
    if(!loaded){
        m_actorPod->post(mpk.fromAddr(), AM_ERROR);
        co_return;
    }

    if(amLM.waitActivated){
        if(const auto loadMpk = co_await m_actorPod->send(amLM.mapUID, AM_WAITACTIVATED); loadMpk.type() != AM_WAITACTIVATEDOK){
            m_actorPod->post(mpk.fromAddr(), AM_ERROR);
            co_return;
        }
    }

    AMLoadMapOK amLMOK;
    std::memset(&amLMOK, 0, sizeof(amLMOK));

    amLMOK.newLoad = newLoad;
    m_actorPod->post(mpk.fromAddr(), {AM_LOADMAPOK, amLMOK});

    if(newLoad){
        g_server->addLog(LOGTYPE_INFO, "Load map %d on peer %zu successfully", to_d(uidf::getMapID(amLM.mapUID)), uidf::peerIndex(UID()));
    }
}

corof::awaitable<> PeerCore::on_AM_CLOSEMAP(const ActorMsgPack &mpk)
{
    // map may run on peer
    // but is manageed on service core

    // if servicecore forward close map request here
    // means 1. this mapUID exists in service core's map list
    //       2. this mapUID is running on this peer core

    const auto amCM = mpk.conv<AMCloseMap>();

    fflassert(uidf::isServiceCore(mpk.from()));
    fflassert(uidf::isMap(amCM.mapUID));
    fflassert(uidsf::isLocalUID(amCM.mapUID));
    fflassert(m_mapList.contains(amCM.mapUID));

    switch(const auto rmpk = co_await m_actorPod->send(amCM.mapUID, {AM_CLOSEMAP, amCM}); rmpk.type()){
        case AM_CLOSEMAPOK:
            {
                m_mapList.erase(amCM.mapUID);
                AMCloseMapOK amCMOK;
                std::memset(&amCMOK, 0, sizeof(amCMOK));

                amCMOK.hasMap = true;
                m_actorPod->post(mpk.fromAddr(), {AM_CLOSEMAPOK, amCMOK});
                co_return;
            }
        default:
            {
                AMCloseMapError amCME;
                std::memset(&amCME, 0, sizeof(amCME));

                amCME.hasMap = true;
                m_actorPod->post(mpk.fromAddr(), {AM_CLOSEMAPERROR, amCME});
                co_return;
            }
    }
}
