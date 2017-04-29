/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 04/28/2017 23:49:12
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
#include "serverconfigurewindow.hpp"

extern ServerConfigureWindow *g_ServerConfigureWindow;
ServerMap::ServerMap(ServiceCore *pServiceCore, uint32_t nMapID)
    : ActiveObject()
    , m_ID(nMapID)
    , m_Mir2xMapData((std::string(g_ServerConfigureWindow->GetMapPath()) + SYS_MAPFILENAME(nMapID)).c_str())
    , m_Metronome(nullptr)
    , m_ServiceCore(pServiceCore)
    , m_CellRecordV2D()
    , m_ObjectV2D()
{
    ResetType(TYPE_INFO,    TYPE_UTILITY);
    ResetType(TYPE_UTILITY, TYPE_SERVERMAP);

    if(m_Mir2xMapData.Valid()){
        m_ObjectV2D.clear();

        m_ObjectV2D.resize(m_Mir2xMapData.W());
        for(auto &rstObjectLine: m_ObjectV2D){
            rstObjectLine.resize(m_Mir2xMapData.H());
        }

        m_CellRecordV2D.resize(m_Mir2xMapData.W());
        for(auto &rstStateLine: m_CellRecordV2D){
            rstStateLine.resize(m_Mir2xMapData.H());
        }
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Load map failed: ID = %d, Name = %s", nMapID, SYS_MAPFILENAME(nMapID) ? SYS_MAPFILENAME(nMapID) : "");
        g_MonoServer->Restart();
    }

    for(auto stLoc: SYS_MAPSWITCHLOC(nMapID)){
        if(ValidC(stLoc.X, stLoc.Y)){
            m_CellRecordV2D[stLoc.X][stLoc.Y].MapID = stLoc.MapID;
        }
    }
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
        case MPK_ACTION:
            {
                On_MPK_ACTION(rstMPK, rstFromAddr);
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
        case MPK_PULLCOINFO:
            {
                On_MPK_PULLCOINFO(rstMPK, rstFromAddr);
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_FATAL, "unsupported message: %s", rstMPK.Name());
                g_MonoServer->Restart();
                break;
            }
    }
}

bool ServerMap::CanMove(int nX, int nY)
{
    if(GroundValid(nX, nY)){
        for(auto pObject: m_ObjectV2D[nX][nY]){
            if(pObject && pObject->Active() && ((ActiveObject *)(pObject))->Type(TYPE_CHAR)){
                return false;
            }
        }
        return true;
    }
    return false;
}
