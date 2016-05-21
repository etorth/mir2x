/*
 * =====================================================================================
 *
 *       Filename: servicecoreop.cpp
 *        Created: 05/03/2016 21:29:58
 *  Last Modified: 05/20/2016 18:03:54
 *
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

#include "player.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"
#include "serverconfigurewindow.hpp"

// ServiceCore accepts net packages from *many* sessions and based on it to create
// the player object for a one to one map
//
// So servicecore <-> session is 1 to N, means we have to put put pointer of session
// in the net package otherwise we can't find the session even we have session's 
// address, session is a sync-driver, even we have it's address we can't find it
//
void ServiceCore::On_MPK_NETPACKAGE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMServiceCoreNetPackage stAMSCNP;
    std::memcpy(&stAMSCNP, rstMPK.Data(), sizeof(stAMSCNP));

    OperateNet(stAMSCNP.Session, stAMSCNP.Type, stAMSCNP.Data, stAMSCNP.DataLen);
}

void ServiceCore::On_MPK_ADDMONSTER(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMAddMonster stAMAM;
    std::memcpy(&stAMAM, rstMPK.Data(), sizeof(stAMAM));

    if(m_MapRecordM.find(stAMAM.MapID) == m_MapRecordM.end()){
        LoadMap(stAMAM.MapID);
    }

    auto fnOPR = [this, rstFromAddr](const MessagePack &rstRMPK, const Theron::Address &){
        switch(rstRMPK.Type()){
            case MPK_OK:
                {
                    m_ActorPod->Forward(MPK_OK, rstFromAddr);
                    break;
                }
            default:
                {
                    m_ActorPod->Forward(MPK_ERROR, rstFromAddr);
                    break;
                }
        }
    };
    m_ActorPod->Forward({MPK_ADDMONSTER, rstMPK.Data(),
            rstMPK.DataLen()}, m_MapRecordM[stAMAM.MapID].PodAddress, fnOPR);
}

void ServiceCore::On_MPK_NEWCONNECTION(const MessagePack &, const Theron::Address &rstFromAddr)
{
    extern ServerConfigureWindow *g_ServerConfigureWindow;
    bool bConnectionOK = true;
    if(false
            || (int)m_PlayerRecordM.size() >= g_ServerConfigureWindow->MaxPlayerCount()
            || false){ // put all criteria to check here
        bConnectionOK = false;
    }
    m_ActorPod->Forward(bConnectionOK ? MPK_OK : MPK_REFUSE, rstFromAddr);
}

void ServiceCore::On_MPK_LOGIN(const MessagePack &rstMPK, const Theron::Address &)
{
    AMLogin stAML;
    std::memcpy(&stAML, rstMPK.Data(), sizeof(stAML));

    extern MonoServer *g_MonoServer;
    auto pNewPlayer = new Player(m_CurrUID++,
            g_MonoServer->GetTimeTick(), stAML.GUID, stAML.SID);

    // ... add all dress, inventory, weapon here
    // ... add all position/direction/map/state here

    AMNewPlayer stAMNP;
    stAMNP.Data = (void *)pNewPlayer;

    if(m_MapRecordM.find(stAML.MapID) == m_MapRecordM.end()){
        // load map
    }
    m_ActorPod->Forward({MPK_NEWPLAYER, stAMNP}, m_MapRecordM[stAML.MapID].PodAddress);
}

void ServiceCore::On_MPK_PLAYERPHATOM(
        const MessagePack &rstMPK, const Theron::Address &)
{
    AMPlayerPhantom stAMPP;
    std::memcpy(&stAMPP, rstMPK.Data(), sizeof(stAMPP));

    if(m_PlayerRecordM.find(stAMPP.GUID) != m_PlayerRecordM.end()){
    }
}
