/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 03/23/2017 16:03:46
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

ServerMap::ServerMap(ServiceCore *pServiceCore, uint32_t nMapID)
    : ActiveObject()
    , m_ID(nMapID)
    , m_Mir2xMapData()
    , m_Metronome(nullptr)
    , m_ServiceCore(pServiceCore)
{
    ResetType(TYPE_INFO,    TYPE_UTILITY);
    ResetType(TYPE_UTILITY, TYPE_SERVERMAP);

    Load("./test.map");
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
        case MPK_LEAVE:
            {
                On_MPK_LEAVE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_TRYMOVE:
            {
                On_MPK_TRYMOVE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK, rstFromAddr);
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
        case MPK_TRYSPACEMOVE:
            {
                On_MPK_TRYSPACEMOVE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_ADDCHAROBJECT:
            {
                On_MPK_ADDCHAROBJECT(rstMPK, rstFromAddr);
                break;
            }
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
        for(auto pObject: m_ObjectV2D[nX][nY]){
            if(pObject->Active() && ((ActiveObject *)(pObject))->Type(TYPE_CHAR)){
                return false;
            }
        }
        return true;
    }
    return false;
}
