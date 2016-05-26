/*
 * =====================================================================================
 *
 *       Filename: servicecore.cpp
 *        Created: 04/22/2016 18:16:53
 *  Last Modified: 05/25/2016 18:31:48
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
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"

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
        case MPK_ADDMONSTER:
            {
                On_MPK_ADDMONSTER(rstMPK, rstAddr);
                break;
            }
        case MPK_NEWCONNECTION:
            {
                On_MPK_NEWCONNECTION(rstMPK, rstAddr);
                break;
            }
        case MPK_LOGIN:
            {
                On_MPK_LOGIN(rstMPK, rstAddr);
                break;
            }
        case MPK_PLAYERPHATOM:
            {
                On_MPK_PLAYERPHATOM(rstMPK, rstAddr);
                break;
            }
        default:
            {
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

    m_ActorPod->Forward(MPK_HI, m_MapRecordM[nMapID].PodAddress);

    return true;
}
