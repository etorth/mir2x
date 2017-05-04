/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 05/04/2017 01:00:43
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
    , m_UIDRecordV2D()
{
    if(m_Mir2xMapData.Valid()){
        m_UIDRecordV2D.clear();

        m_UIDRecordV2D.resize(m_Mir2xMapData.W());
        for(auto &rstRecordLine: m_UIDRecordV2D){
            rstRecordLine.resize(m_Mir2xMapData.H());
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

    auto fnRegisterClass = [this]() -> void {
        if(!RegisterClass<ServerMap, ActiveObject>()){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Class registration for <ServerMap, ActiveObject> failed");
            g_MonoServer->Restart();
        }
    };
    static std::once_flag stFlag;
    std::call_once(stFlag, fnRegisterClass);
}

void ServerMap::Operate(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    switch(rstMPK.Type()){
        case MPK_HI:
            {
                On_MPK_HI(rstMPK, rstFromAddr);
                break;
            }
        case MPK_TRYLEAVE:
            {
                On_MPK_TRYLEAVE(rstMPK, rstFromAddr);
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
        case MPK_TRYMAPSWITCH:
            {
                On_MPK_TRYMAPSWITCH(rstMPK, rstFromAddr);
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
        for(auto nUID: m_UIDRecordV2D[nX][nY]){
            extern MonoServer *g_MonoServer;
            if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                if(stUIDRecord.ClassFrom<CharObject>()){
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}
