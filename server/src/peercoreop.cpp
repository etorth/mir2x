#include "uidf.hpp"
#include "uidsf.hpp"
#include "serverguard.hpp"
#include "player.hpp"
#include "npchar.hpp"
#include "monster.hpp"
#include "peercore.hpp"
#include "serdesmsg.hpp"
#include "monoserver.hpp"

extern MonoServer *g_monoServer;

void PeerCore::on_AM_ADDCO(const ActorMsgPack &mpk)
{
    // always create CO if request received
    // it's sender's responsibility to figure out to forward the request to which peer

    const auto sdICO = mpk.deserialize<SDInitCharObject>();
    const auto fnAddCO = [sdICO, fromAddr = mpk.fromAddr(), this]()
    {
        std::string err;
        try{
            auto coPtr = std::visit(VarDispatcher
            {
                [this](const SDInitGuard   &sdIG  ) { return static_cast<CharObject *>(addGuard  (sdIG  )); },
                [this](const SDInitPlayer  &sdIP  ) { return static_cast<CharObject *>(addPlayer (sdIP  )); },
                [this](const SDInitNPChar  &sdINPC) { return static_cast<CharObject *>(addNPChar (sdINPC)); },
                [this](const SDInitMonster &sdIM  ) { return static_cast<CharObject *>(addMonster(sdIM  )); },
            },

            sdICO);

            AMUID amUID;
            std::memset(&amUID, 0, sizeof(amUID));

            amUID.uid = coPtr->UID();
            m_actorPod->forward(fromAddr, {AM_UID, amUID});
        }
        catch(const std::exception &e){
            err = e.what();
        }
        catch(...){
            err = "unknown error";
        }

        g_monoServer->addLog(LOGTYPE_WARNING, "Failed to create char object: %s", to_cstr(err));
        m_actorPod->forward(fromAddr, AM_ERROR);
    };

    const auto mapUID = std::visit(VarDispatcher
    {
        [this](const SDInitGuard   &sdIG  ) { return sdIG  .mapUID; },
        [this](const SDInitPlayer  &sdIP  ) { return sdIP  .mapUID; },
        [this](const SDInitNPChar  &sdINPC) { return sdINPC.mapUID; },
        [this](const SDInitMonster &sdIM  ) { return sdIM  .mapUID; },
    },

    sdICO);

    AMLoadMap amLM;
    std::memset(&amLM, 0, sizeof(amLM));

    amLM.mapUID = mapUID;
    m_actorPod->forward(uidf::getServiceCoreUID(), {AM_LOADMAP, amLM}, [fromAddr = mpk.fromAddr(), fnAddCO, this](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_LOADMAPOK:
                {
                    fnAddCO();
                    break;
                }
            default:
                {
                    m_actorPod->forward(fromAddr, AM_ERROR);
                    break;
                }
        }
    });
}

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
        AMLoadMapOK amLMOK;
        std::memset(&amLMOK, 0, sizeof(amLMOK));

        amLMOK.newLoad = newLoad;
        m_actorPod->forward(mpk.fromAddr(), {AM_LOADMAPOK, amLMOK});
    }
    else{
        m_actorPod->forward(mpk.fromAddr(), AM_ERROR);
    }
}
