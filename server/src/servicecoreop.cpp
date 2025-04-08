#include <string>
#include <type_traits>

#include "player.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"
#include "mapbindb.hpp"

extern MapBinDB *g_mapBinDB;
extern MonoServer *g_monoServer;

// ServiceCore accepts net packages from *many* sessions and based on it to create
// the player object for a one to one map
//
// So servicecore <-> session is 1 to N, means we have to put put pointer of session
// in the net package otherwise we can't find the session even we have session's
// address, session is a sync-driver, even we have it's address we can't find it
//
void ServiceCore::on_AM_RECVPACKAGE(const ActorMsgPack &mpk)
{
    /* const */ auto amRP = mpk.conv<AMRecvPackage>();
    operateNet(amRP.channID, amRP.package.type, amRP.package.buf(), amRP.package.size, amRP.package.resp);
    freeActorDataPackage(&(amRP.package));
}

void ServiceCore::on_AM_METRONOME(const ActorMsgPack &)
{
}

void ServiceCore::on_AM_REGISTERQUEST(const ActorMsgPack &mpk)
{
    const auto sdRQ = mpk.deserialize<SDRegisterQuest>();
    m_questList[mpk.from()] = sdRQ;
}

void ServiceCore::on_AM_ADDCO(const ActorMsgPack &rstMPK)
{
    const auto amACO = rstMPK.conv<AMAddCharObject>();
    if(!amACO.mapUID){
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
        return;
    }

    if(!g_mapBinDB->retrieve(uidf::getMapID(amACO.mapUID))->validC(amACO.x, amACO.y) && amACO.strictLoc){
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
        return;
    }

    loadMap(uidf::getMapID(amACO.mapUID), [amACO, rstMPK, this](bool)
    {
        m_actorPod->forward(amACO.mapUID, {AM_ADDCO, amACO}, [this, amACO, rstMPK](const ActorMsgPack &rstRMPK)
        {
            m_actorPod->forward(rstMPK.from(), {rstRMPK.type(), rstRMPK.data()}, rstMPK.seqID());
        });
    },

    [rstMPK, this]()
    {
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
    });
}

void ServiceCore::on_AM_ADDMAP(const ActorMsgPack &rstMPK)
{
    const auto amAM = rstMPK.conv<AMAddMap>();
    loadMap(amAM.mapID, [amAM, rstMPK, this](bool newLoad)
    {
        AMAddMapOK amAMOK;
        std::memset(&amAMOK, 0, sizeof(amAMOK));

        amAMOK.mapID   = amAM.mapID;
        amAMOK.newLoad = newLoad;

        m_actorPod->forward(rstMPK.from(), {AM_ADDMAPOK, amAMOK}, rstMPK.seqID());
    },

    [rstMPK, this]()
    {
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
    });
}

void ServiceCore::on_AM_QUERYMAPLIST(const ActorMsgPack &rstMPK)
{
    AMMapList amML;
    std::memset(&amML, 0, sizeof(amML));

    size_t nIndex = 0;
    for(const auto &[mapID, ops]: m_loadMapOps){
        if(nIndex < std::extent_v<decltype(amML.MapList)>){
            amML.MapList[nIndex++] = mapID;
        }
        else{
            throw fflerror("Need larger map list size in AMMapList");
        }
    }
    m_actorPod->forward(rstMPK.from(), {AM_MAPLIST, amML}, rstMPK.seqID());
}

void ServiceCore::on_AM_LOADMAP(const ActorMsgPack &mpk)
{
    const auto amLM = mpk.conv<AMLoadMap>();
    loadMap(amLM.mapID, [amLM, mpk, this](bool)
    {
        AMLoadMapOK amLMOK;
        std::memset(&amLMOK, 0, sizeof(amLMOK));

        amLMOK.uid = uidf::getMapBaseUID(amLM.mapID);
        m_actorPod->forward(mpk.fromAddr(), {AM_LOADMAPOK, amLMOK});
    },

    [mpk, this]()
    {
        m_actorPod->forward(mpk.fromAddr(), AM_LOADMAPERROR);
    });
}

void ServiceCore::on_AM_QUERYCOCOUNT(const ActorMsgPack &rstMPK)
{
    const auto amQCOC = rstMPK.conv<AMQueryCOCount>();
    const auto checkCount = [&amQCOC, this]()
    {
        if(amQCOC.mapID){
            if(auto p = m_loadMapOps.find(amQCOC.mapID); p != m_loadMapOps.end() && !p->second.pending){
                return 1;
            }
            else{
                return 0;
            }
        }
        return -1;
    }();

    switch(checkCount){
        case 0:
            {
                m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
                return;
            }
        case 1:
            {
                m_actorPod->forward(uidf::getMapBaseUID(amQCOC.mapID), {AM_QUERYCOCOUNT, amQCOC}, [this, rstMPK](const ActorMsgPack &rstRMPK)
                {
                    switch(rstRMPK.type()){
                        case AM_COCOUNT:
                            {
                                m_actorPod->forward(rstMPK.from(), {AM_COCOUNT, rstRMPK.data(), rstRMPK.size()}, rstMPK.seqID());
                                return;
                            }
                        case AM_ERROR:
                        default:
                            {
                                m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
                                return;
                            }
                    }
                });
                return;
            }
        default:
            {
                std::set<uint32_t> mapList;
                for(const auto &[mapID, op]: m_loadMapOps){
                    if(!op.pending){
                        mapList.insert(mapID);
                    }
                }

                struct SharedState
                {
                    bool error = false;
                    int  count = 0;
                    int  check = 0;
                };

                auto sharedState = std::make_shared<SharedState>();
                sharedState->check = to_d(mapList.size());

                auto fnOnResp = [sharedState, this, rstMPK](const ActorMsgPack &rstRMPK)
                {
                    switch(rstRMPK.type()){
                        case AM_COCOUNT:
                            {
                                if(!sharedState->error){
                                    auto amCOC = rstRMPK.conv<AMCOCount>();
                                    if(sharedState->check == 1){
                                        amCOC.Count += sharedState->count;
                                        m_actorPod->forward(rstMPK.from(), {AM_COCOUNT, amCOC}, rstMPK.seqID());
                                    }
                                    else{
                                        sharedState->check--;
                                        sharedState->count += to_d(amCOC.Count);
                                    }
                                }
                                return;
                            }
                        case AM_ERROR:
                        default:
                            {
                                sharedState->error = true;
                                m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
                                return;
                            }
                    }
                };

                for(const auto mapID: mapList){
                    m_actorPod->forward(uidf::getMapBaseUID(mapID), {AM_QUERYCOCOUNT, amQCOC}, fnOnResp);
                }
                return;
            }
    }
}

void ServiceCore::on_AM_MODIFYQUESTTRIGGERTYPE(const ActorMsgPack &mpk)
{
    const auto amMQTT = mpk.conv<AMModifyQuestTriggerType>();

    fflassert(amMQTT.type >= SYS_ON_BEGIN, amMQTT.type);
    fflassert(amMQTT.type <  SYS_ON_END  , amMQTT.type);

    if(amMQTT.enable){
        m_questTriggerList[amMQTT.type].insert(mpk.from());
    }
    else{
        m_questTriggerList[amMQTT.type].erase(mpk.from());
    }

    // give an response
    // helps quest side to confirm that enable/disable is done
    m_actorPod->forward(mpk.fromAddr(), AM_OK);
}

void ServiceCore::on_AM_QUERYQUESTTRIGGERLIST(const ActorMsgPack &mpk)
{
    const auto amQQTL = mpk.conv<AMQueryQuestTriggerList>();

    fflassert(amQQTL.type >= SYS_ON_BEGIN, amQQTL.type);
    fflassert(amQQTL.type <  SYS_ON_END  , amQQTL.type);

    std::vector<uint64_t> uidList;
    if(const auto p = m_questTriggerList.find(amQQTL.type); p != m_questTriggerList.end()){
        uidList.assign(p->second.begin(), p->second.end());
    }

    m_actorPod->forward(mpk.fromAddr(), {AM_OK, cerealf::serialize(uidList)});
}

void ServiceCore::on_AM_QUERYQUESTUID(const ActorMsgPack &mpk)
{
    const auto sdQQUID = mpk.deserialize<SDQueryQuestUID>();

    uint64_t questUID = 0;
    for(const auto &[uid, sdRQ]: m_questList){
        if(sdQQUID.name == sdRQ.name){
            questUID = uid;
            break;
        }
    }

    AMUID amUID;
    std::memset(&amUID, 0, sizeof(amUID));

    amUID.UID = questUID;
    m_actorPod->forward(mpk.fromAddr(), {AM_UID, amUID});
}

void ServiceCore::on_AM_QUERYQUESTUIDLIST(const ActorMsgPack &mpk)
{
    SDUIDList uidList;
    uidList.reserve(m_questList.size());

    for(const auto &[uid, sdRQ]: m_questList){
        uidList.push_back(uid);
    }

    m_actorPod->forward(mpk.fromAddr(), {AM_UIDLIST, cerealf::serialize(uidList)});
}

void ServiceCore::on_AM_BADCHANNEL(const ActorMsgPack &mpk)
{
    const auto amBC = mpk.conv<AMBadChannel>();
    m_dbidList.erase(amBC.channID);
}
