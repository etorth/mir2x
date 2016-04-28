/*
 * =====================================================================================
 *
 *       Filename: servicecore.cpp
 *        Created: 04/22/2016 18:16:53
 *  Last Modified: 04/28/2016 00:38:50
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

#include <system_error>

#include "actorpod.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"

#include "serverconfigurewindow.hpp"

static int s_Count = 0;

ServiceCore::ServiceCore()
    : Transponder()
{
    s_Count++;
    if(s_Count > 1){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "one service core please");
        throw std::error_code();
    }
}

ServiceCore::~ServiceCore()
{
    s_Count--;
}

void ServiceCore::Operate(const MessagePack &rstMPK, const Theron::Address &rstAddr)
{
    switch(rstMPK.Type()){
        case MPK_NEWCONNECTION:
            {
                extern ServerConfigureWindow *g_ServerConfigureWindow;
                bool bConnectionOK = true;
                if(false
                        || m_PlayerV.size() >= (size_t)g_ServerConfigureWindow->MaxPlayerCount()
                        || false){ // put all criteria to check here
                    bConnectionOK = false;
                }
                m_ActorPod->Send(MessagePack(bConnectionOK ? MPK_OK : MPK_REFUSE), rstAddr);
                break;
            }
        case MPK_LOGIN:
            {
                AMLogin stAML;
                std::memcpy(&stAML, rstMPK.Data(), sizeof(stAML));

                extern MonoServer *g_MonoServer;
                auto pNewPlayer = new Player(stAML.SID, stAML.GUID, 
                        g_MonoServer->GetTimeTick(), );

                // ... add all dress, inventory, weapon here
                // ... add all position/direction/map/state here

                AMNewPlayer stAMNP = {
                    .Data = (void *)pNewPlayer,
                };

                if(m_MapRecordMap.find(stAML.MapID) == m_MapRecordMap.end()){
                    // load map
                }
                m_ActorPod->Send(MessagePack(MPK_NEWPLAYER,
                            stMNP), m_MapRecordMap[stAML.MapID].PodAddress);
                break;
            }
        case MPK_PLAYERPHATOM:
            {
                AMPlayerPhantom stAMPP;
                std::memcpy(&stAMPP, rstMPK.Data(), sizeof(stAMPP));

                if(m_PlayerRecordMap.find(stAMPP.GUID) != m_PlayerRecordMap.end()){
                }
                break;
            }
    }
}

bool ServiceCore::LoadMap(uint32_t nMapID)
{
    if(nMapID == 0){ return false; }
}
