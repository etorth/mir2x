#include <string>
#include <type_traits>

#include "uidsf.hpp"
#include "player.hpp"
#include "actorpod.hpp"
#include "server.hpp"
#include "servicecore.hpp"
#include "mapbindb.hpp"

extern MapBinDB *g_mapBinDB;
extern Server *g_server;

// ServiceCore accepts net packages from *many* sessions and based on it to create
// the player object for a one to one map
//
// So servicecore <-> session is 1 to N, means we have to put put pointer of session
// in the net package otherwise we can't find the session even we have session's
// address, session is a sync-driver, even we have it's address we can't find it
//
corof::awaitable<> ServiceCore::on_AM_RECVPACKAGE(const ActorMsgPack &mpk)
{
    /* const */ auto amRP = mpk.conv<AMRecvPackage>();
    operateNet(amRP.channID, amRP.package.type, amRP.package.buf(), amRP.package.size, amRP.package.resp);
    freeActorDataPackage(&(amRP.package));
}

corof::awaitable<> ServiceCore::on_AM_METRONOME(const ActorMsgPack &)
{
}

corof::awaitable<> ServiceCore::on_AM_REGISTERQUEST(const ActorMsgPack &mpk)
{
    const auto sdRQ = mpk.deserialize<SDRegisterQuest>();
    m_questList[mpk.from()] = sdRQ;
}

corof::awaitable<> ServiceCore::on_AM_QUERYMAPLIST(const ActorMsgPack &rstMPK)
{
    AMMapList amML;
    std::memset(&amML, 0, sizeof(amML));

    size_t nIndex = 0;
    for(const auto mapUID: m_mapList){
        if(nIndex < std::extent_v<decltype(amML.MapList)>){
            amML.MapList[nIndex++] = uidf::getMapID(mapUID);
        }
        else{
            throw fflerror("Need larger map list size in AMMapList");
        }
    }
    m_actorPod->post(rstMPK.fromAddr(), {AM_MAPLIST, amML});
}

corof::awaitable<> ServiceCore::on_AM_LOADMAP(const ActorMsgPack &mpk)
{
    const auto amLM = mpk.conv<AMLoadMap>();
    requestLoadMap(amLM.mapUID, [mpk, this](bool newLoad)
    {
        AMLoadMapOK amLMOK;
        std::memset(&amLMOK, 0, sizeof(amLMOK));

        amLMOK.newLoad = newLoad;
        m_actorPod->post(mpk.fromAddr(), {AM_LOADMAPOK, amLMOK});
    },

    [mpk, this]()
    {
        m_actorPod->post(mpk.fromAddr(), AM_ERROR);
    });
}

corof::awaitable<> ServiceCore::on_AM_QUERYCOCOUNT(const ActorMsgPack &rstMPK)
{
    const auto amQCOC = rstMPK.conv<AMQueryCOCount>();
    const auto checkCount = [&amQCOC, this]()
    {
        if(amQCOC.mapUID){
            if(m_mapList.contains(amQCOC.mapUID)){
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
                m_actorPod->post(rstMPK.fromAddr(), AM_ERROR);
                return;
            }
        case 1:
            {
                m_actorPod->send(amQCOC.mapUID, {AM_QUERYCOCOUNT, amQCOC}, [this, rstMPK](const ActorMsgPack &rstRMPK)
                {
                    switch(rstRMPK.type()){
                        case AM_COCOUNT:
                            {
                                m_actorPod->post(rstMPK.fromAddr(), {AM_COCOUNT, rstRMPK.data(), rstRMPK.size()});
                                return;
                            }
                        case AM_ERROR:
                        default:
                            {
                                m_actorPod->post(rstMPK.fromAddr(), AM_ERROR);
                                return;
                            }
                    }
                });
                return;
            }
        default:
            {
                struct SharedState
                {
                    bool   error = false;
                    size_t count = 0;
                    int    check = 0;
                };

                auto sharedState = std::make_shared<SharedState>();
                sharedState->check = to_d(m_mapList.size());

                auto fnOnResp = [sharedState, this, rstMPK](const ActorMsgPack &rstRMPK)
                {
                    switch(rstRMPK.type()){
                        case AM_COCOUNT:
                            {
                                if(!sharedState->error){
                                    auto amCOC = rstRMPK.conv<AMCOCount>();
                                    if(sharedState->check == 1){
                                        amCOC.Count += sharedState->count;
                                        m_actorPod->post(rstMPK.fromAddr(), {AM_COCOUNT, amCOC});
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
                                m_actorPod->post(rstMPK.fromAddr(), AM_ERROR);
                                return;
                            }
                    }
                };

                for(const auto mapUID: m_mapList){
                    m_actorPod->post(mapUID, {AM_QUERYCOCOUNT, amQCOC}, fnOnResp);
                }
                return;
            }
    }
}

corof::awaitable<> ServiceCore::on_AM_MODIFYQUESTTRIGGERTYPE(const ActorMsgPack &mpk)
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
    m_actorPod->post(mpk.fromAddr(), AM_OK);
}

corof::awaitable<> ServiceCore::on_AM_QUERYQUESTTRIGGERLIST(const ActorMsgPack &mpk)
{
    const auto amQQTL = mpk.conv<AMQueryQuestTriggerList>();

    fflassert(amQQTL.type >= SYS_ON_BEGIN, amQQTL.type);
    fflassert(amQQTL.type <  SYS_ON_END  , amQQTL.type);

    std::vector<uint64_t> uidList;
    if(const auto p = m_questTriggerList.find(amQQTL.type); p != m_questTriggerList.end()){
        uidList.assign(p->second.begin(), p->second.end());
    }

    m_actorPod->post(mpk.fromAddr(), {AM_OK, cerealf::serialize(uidList)});
}

corof::awaitable<> ServiceCore::on_AM_QUERYQUESTUID(const ActorMsgPack &mpk)
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

    amUID.uid = questUID;
    m_actorPod->post(mpk.fromAddr(), {AM_UID, amUID});
}

corof::awaitable<> ServiceCore::on_AM_QUERYQUESTUIDLIST(const ActorMsgPack &mpk)
{
    SDUIDList uidList;
    uidList.reserve(m_questList.size());

    for(const auto &[uid, sdRQ]: m_questList){
        uidList.push_back(uid);
    }

    m_actorPod->post(mpk.fromAddr(), {AM_UIDLIST, cerealf::serialize(uidList)});
}

corof::awaitable<> ServiceCore::on_AM_BADCHANNEL(const ActorMsgPack &mpk)
{
    const auto amBC = mpk.conv<AMBadChannel>();
    m_dbidList.erase(amBC.channID);
}
