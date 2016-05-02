/*
 * =====================================================================================
 *
 *       Filename: servicecore.cpp
 *        Created: 04/22/2016 18:16:53
 *  Last Modified: 05/01/2016 23:12:33
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

#include "player.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"
#include "serverconfigurewindow.hpp"

static int s_Count = 0;

ServiceCore::ServiceCore()
    : Transponder()
    , m_CurrUID(1)
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
        case MPK_ADDMONSTER:
            {
                AMAddMonster stAMAM;
                std::memcpy(&stAMAM, rstMPK.Data(), sizeof(stAMAM));

                if(m_MapRecordM.find(stAMAM.MapID) == m_MapRecordM.end()){
                    LoadMap(stAMAM.MapID);
                }

                auto fnOPR = [this, rstCopyAddr = rstAddr](
                        const MessagePack &rstRMPK, const Theron::Address &)
                {
                    switch(rstRMPK.Type()){
                        case MPK_OK:
                            {
                                Send(MessagePack(MPK_OK), rstCopyAddr);
                                break;
                            }
                        default:
                            {
                                Send(MessagePack(MPK_ERROR), rstCopyAddr);
                                break;
                            }
                    }
                };
                Send(rstMPK, m_MapRecordM[stAMAM.MapID].PodAddress, fnOPR);
                break;
            }
        case MPK_NEWCONNECTION:
            {
                extern ServerConfigureWindow *g_ServerConfigureWindow;
                bool bConnectionOK = true;
                if(false
                        || (int)m_PlayerRecordM.size() >= g_ServerConfigureWindow->MaxPlayerCount()
                        || false){ // put all criteria to check here
                    bConnectionOK = false;
                }
                Send(MessagePack(bConnectionOK ? MPK_OK : MPK_REFUSE), rstAddr);
                break;
            }
        case MPK_LOGIN:
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
                Send(MessagePack(MPK_NEWPLAYER, stAMNP), m_MapRecordM[stAML.MapID].PodAddress);
                break;
            }
        case MPK_PLAYERPHATOM:
            {
                AMPlayerPhantom stAMPP;
                std::memcpy(&stAMPP, rstMPK.Data(), sizeof(stAMPP));

                if(m_PlayerRecordM.find(stAMPP.GUID) != m_PlayerRecordM.end()){
                }
                break;
            }
    }
}

bool ServiceCore::LoadMap(uint32_t nMapID)
{
    if(nMapID == 0){ return false; }

    ServerMap *pNewMap = new ServerMap(nMapID);

    m_MapRecordM[nMapID].MapID      = nMapID;
    m_MapRecordM[nMapID].Map        = pNewMap;
    m_MapRecordM[nMapID].PodAddress = pNewMap->Activate();

    return true;
}
