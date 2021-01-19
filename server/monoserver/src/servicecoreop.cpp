/*
 * =====================================================================================
 *
 *       Filename: servicecoreop.cpp
 *        Created: 05/03/2016 21:29:58
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */
#include <string>
#include <type_traits>

#include "player.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"

extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;

// ServiceCore accepts net packages from *many* sessions and based on it to create
// the player object for a one to one map
//
// So servicecore <-> session is 1 to N, means we have to put put pointer of session
// in the net package otherwise we can't find the session even we have session's
// address, session is a sync-driver, even we have it's address we can't find it
//
void ServiceCore::on_MPK_RECVPACKAGE(const MessagePack &mpk)
{
    /* const */ auto amRP = mpk.conv<AMRecvPackage>();
    operateNet(amRP.channID, amRP.package.type, amRP.package.buf(), amRP.package.size);
    freeNetPackage(&(amRP.package));
}

void ServiceCore::on_MPK_METRONOME(const MessagePack &)
{
}

void ServiceCore::on_MPK_ADDCHAROBJECT(const MessagePack &rstMPK)
{
    const auto amACO = rstMPK.conv<AMAddCharObject>();
    if(!amACO.mapID){
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    auto pMap = retrieveMap(amACO.mapID);

    if(!pMap){
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    if(!pMap->In(amACO.mapID, amACO.x, amACO.y) && amACO.strictLoc){
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    m_actorPod->forward(pMap->UID(), {MPK_ADDCHAROBJECT, amACO}, [this, amACO, rstMPK](const MessagePack &rstRMPK)
    {
        switch(rstRMPK.Type()){
            case MPK_OK:
                {
                    m_actorPod->forward(rstMPK.from(), MPK_OK, rstMPK.ID());
                    break;
                }
            default:
                {
                    m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
                    break;
                }
        }
    });
}

void ServiceCore::on_MPK_QUERYMAPLIST(const MessagePack &rstMPK)
{
    AMMapList amML;
    std::memset(&amML, 0, sizeof(amML));

    size_t nIndex = 0;
    for(auto pMap: m_mapList){
        if(pMap.second && pMap.second->ID()){
            if(nIndex < std::extent_v<decltype(amML.MapList)>){
                amML.MapList[nIndex++] = pMap.second->ID();
            }else{
                throw fflerror("Need larger map list size in AMMapList");
            }
        }
    }
    m_actorPod->forward(rstMPK.from(), {MPK_MAPLIST, amML}, rstMPK.ID());
}

void ServiceCore::on_MPK_QUERYMAPUID(const MessagePack &mpk)
{
    const auto amQMUID = mpk.conv<AMQueryMapUID>();
    const auto mapPtr = retrieveMap(amQMUID.MapID);

    if(!mapPtr){
        m_actorPod->forward(mpk.from(), MPK_ERROR, mpk.ID());
        return;
    }

    AMUID amUID;
    std::memset(&amUID, 0, sizeof(amUID));

    amUID.UID = mapPtr->UID();
    m_actorPod->forward(mpk.from(), {MPK_UID, amUID}, mpk.ID());
}

void ServiceCore::on_MPK_QUERYCOCOUNT(const MessagePack &rstMPK)
{
    AMQueryCOCount amQCOC;
    std::memcpy(&amQCOC, rstMPK.Data(), sizeof(amQCOC));

    int nCheckCount = 0;
    if(amQCOC.MapID){
        if(m_mapList.find(amQCOC.MapID) == m_mapList.end()){
            nCheckCount = 0;
        }else{
            nCheckCount = 1;
        }
    }else{
        nCheckCount = (int)(m_mapList.size());
    }

    switch(nCheckCount){
        case 0:
            {
                m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
                return;
            }
        case 1:
            {
                if(auto pMap = (amQCOC.MapID ? m_mapList[amQCOC.MapID] : m_mapList.begin()->second)){
                    m_actorPod->forward(pMap->UID(), {MPK_QUERYCOCOUNT, amQCOC}, [this, rstMPK](const MessagePack &rstRMPK)
                    {
                        switch(rstRMPK.Type()){
                            case MPK_COCOUNT:
                                {
                                    m_actorPod->forward(rstMPK.from(), {MPK_COCOUNT, rstRMPK.Data(), rstRMPK.DataLen()}, rstMPK.ID());
                                    return;
                                }
                            case MPK_ERROR:
                            default:
                                {
                                    m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
                                    return;
                                }
                        }
                    });
                    return;
                }else{
                    m_mapList.erase(amQCOC.MapID);
                    m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
                    return;
                }
            }
        default:
            {
                // difficult part
                // need send multiple query message and collect them
                // after all collected we need to return the sum, problem:
                // 1. share state
                // 2. error handle

                struct SharedState
                {
                    bool Done;
                    int  CheckCount;
                    int  COCount;

                    SharedState(int nCheckCount)
                        : Done(false)
                        , CheckCount(nCheckCount)
                        , COCount(0)
                    {}
                };

                // current I don't have error handling
                // means if one query didn't get responded it will wait forever
                // to solve this issue, we can install an state hook but for simplity not now

                auto pSharedState = std::make_shared<SharedState>(nCheckCount);
                auto fnOnResp = [pSharedState, this, rstMPK](const MessagePack &rstRMPK)
                {
                    switch(rstRMPK.Type()){
                        case MPK_COCOUNT:
                            {
                                if(pSharedState->Done){
                                    // we get response but shared state shows ``done"
                                    // means more than one error has alreay happened before
                                    // do nothing
                                }else{
                                    // get one more valid response
                                    // need to check if we need to response to sender
                                    AMCOCount amCOC;
                                    std::memcpy(&amCOC, rstRMPK.Data(), sizeof(amCOC));

                                    if(pSharedState->CheckCount == 1){
                                        amCOC.Count += pSharedState->COCount;
                                        m_actorPod->forward(rstMPK.from(), {MPK_COCOUNT, amCOC}, rstMPK.ID());
                                    }else{
                                        pSharedState->CheckCount--;
                                        pSharedState->COCount += (int)(amCOC.Count);
                                    }
                                }
                                return;
                            }
                        case MPK_ERROR:
                        default:
                            {
                                if(pSharedState->Done){
                                    // we get response but shared state shows ``done"
                                    // means more than one error has alreay happened before
                                    // do nothing
                                }else{
                                    // get first error
                                    m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
                                }
                                return;
                            }
                    }
                };

                for(auto p: m_mapList){
                    m_actorPod->forward(p.second->UID(), {MPK_QUERYCOCOUNT, amQCOC}, fnOnResp);
                }
                return;
            }
    }
}

void ServiceCore::on_MPK_BADCHANNEL(const MessagePack &rstMPK)
{
    // channel may go down before bind to one actor
    // then stop it here

    AMBadChannel amBC;
    std::memcpy(&amBC, rstMPK.Data(), sizeof(amBC));

    g_netDriver->Shutdown(amBC.ChannID, false);
}
