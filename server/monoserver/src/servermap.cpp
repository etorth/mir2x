/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 03/22/2017 18:14:00
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

#include <algorithm>

#include "monster.hpp"
#include "actorpod.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "servermap.hpp"
#include "charobject.hpp"
#include "monoserver.hpp"
#include "rotatecoord.hpp"

ServerMap::ServerMap(ServiceCore *pServiceCore, uint32_t nMapID)
    : Transponder()
    , m_ID(nMapID)
    , m_Mir2xMapData()
    , m_Metronome(nullptr)
    , m_ServiceCore(pServiceCore)
{
    m_Metronome = new Metronome(100);
    m_Metronome->Activate(GetAddress());
}

bool ServerMap::Load(const char *szMapFullName)
{
    for(auto &rstRecordLine: m_ObjectV2D){
        for(auto &rstRecordV: rstRecordLine){
            for(auto pObject: rstRecordV){ delete pObject; }
        }
    }

    m_ObjectV2D.clear();

    if(m_Mir2xMapData.Load(szMapFullName)){
        m_ObjectV2D.resize(W());
        for(auto &rstObjectLine: m_ObjectV2D){
            rstObjectLine.resize(H());
        }
    }

    return false;
}

void ServerMap::Operate(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    switch(rstMPK.Type()){
        case MPK_HI:
            {
                On_MPK_HI(rstMPK, rstFromAddr);
                break;
            }
        case MPK_ACTIONSTATE:
            {
                On_MPK_ACTIONSTATE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_UPDATECOINFO:
            {
                On_MPK_UPDATECOINFO(rstMPK, rstFromAddr);
                break;
            }
        // case MPK_ADDMONSTER:
        //     {
        //         On_MPK_ADDMONSTER(rstMPK, rstFromAddr);
        //         break;
        //     }
        // case MPK_NEWMONSTER:
        //     {
        //         On_MPK_NEWMONSTER(rstMPK, rstFromAddr);
        //         break;
        //     }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported message: %s", rstMPK.Name());
                g_MonoServer->Restart();
                break;
            }
    }
}

bool ServerMap::GroundValid(int nX, int nY)
{
    if(ValidC(nX, nY)){
        return true;
    }

    return false;
}
