/*
 * =====================================================================================
 *
 *       Filename: servicecore.cpp
 *        Created: 04/22/2016 18:16:53
 *  Last Modified: 04/23/2016 00:52:11
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

#include "servicecore.hpp"
bool ServiceCore::Operate(const MessagePack &rstMPK, const Theron::Address &rstAddr)
{
    switch(rstMPK.Type()){
        case MPK_NEWCONNECTION:
            {
                extern ServerConfigureWindow *g_ServerConfigureWindow;
                bool bConnectionOK = true;
                if(false
                        || m_PlayerV.size() >= g_ServerConfigureWindow->MaxPlayerCount()
                        || false){ // put all criteria to check here
                    bConnectionOK = false;
                }
                m_ObjectPod->Send(MessagePack(bConnectionOK ? MPK_OK : MPK_REFUSE), rstAddr);
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
                m_ObjectPod->Send(MessagePack(MPK_NEWPLAYER,
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
