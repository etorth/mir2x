/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 05/05/2017 00:36:54
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

ServerMap::ServerPathFinder::ServerPathFinder(ServerMap *pMap, bool bCheckCreature)
    : AStarPathFinder(
            // 1. step check function
            //    assert pMap should be not null
            [pMap, bCheckCreature](int nSrcX, int nSrcY, int nDstX, int nDstY) -> bool {
                switch(LDistance2(nSrcX, nSrcY, nDstX, nDstY)){
                    case 0:
                        {
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid argument: (%d, %d, %d, %d)", nSrcX, nSrcY, nDstX, nDstY);
                            g_MonoServer->Restart();
                            return false;
                        }
                    case 1:
                    case 2:
                        {
                            return bCheckCreature ? pMap->CanMove(nDstX, nDstY) : pMap->GroundValid(nDstX, nDstY);
                        }
                    default:
                        {
                            return false;
                        }
                }
            },

            // 2. move cost function, for directions as following
            //    
            //                  0
            //               7     1
            //             6    x    2
            //               5     3
            //                  4
            //    we put directions 1, 3, 5, 7 as higher cost because for following grids:
            //
            //              A B C D E
            //              F G H I J
            //              K L M N O
            //              P Q R S T
            //
            //    if we make all directions equally cost, then G->I: G -> C -> I
            //    we want the result as G -> H -> I
            [pMap, bCheckCreature](int nSrcX, int nSrcY, int nDstX, int nDstY) -> double {
                switch(LDistance2(nSrcX, nSrcY, nDstX, nDstY)){
                    case 0:
                        {
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid argument: (%d, %d, %d, %d)", nSrcX, nSrcY, nDstX, nDstY);
                            g_MonoServer->Restart();
                            return 10000.0;
                        }
                    case 1:
                        {
                            return (bCheckCreature ? pMap->CanMove(nDstX, nDstY) : pMap->GroundValid(nDstX, nDstY)) ? 1.0 : 10000.0;
                        }
                    case 2:
                        {
                            return (bCheckCreature ? pMap->CanMove(nDstX, nDstY) : pMap->GroundValid(nDstX, nDstY)) ? 1.1 : 10000.0;
                        }
                    default:
                        {
                            return 10000.0;
                        }
                }
            }
      )
{
    if(!pMap){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid argument: ServerMap = %p, CheckCreature = %d", pMap, (int)(bCheckCreature));
        g_MonoServer->Restart();
    }
}

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
            m_CellRecordV2D[stLoc.X][stLoc.Y].UID   = 0;
            m_CellRecordV2D[stLoc.X][stLoc.Y].MapID = stLoc.MapID;
            m_CellRecordV2D[stLoc.X][stLoc.Y].Query = QUERY_NA;
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
        case MPK_PATHFIND:
            {
                On_MPK_PATHFIND(rstMPK, rstFromAddr);
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

bool ServerMap::RandomLocation(int *pX, int *pY)
{
    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){
            if(CanMove(nX, nY)){
                if(pX){ *pX = nX; }
                if(pY){ *pY = nY; }
                return true;
            }
        }
    }
    return false;
}

bool ServerMap::Empty()
{
    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){
            if(GroundValid(nX, nY)){
                if(m_CellRecordV2D[nX][nY].Freezed){
                    return false;
                }
                for(auto nUID: m_UIDRecordV2D[nX][nY]){
                    extern MonoServer *g_MonoServer;
                    if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                        return false;
                    }
                }
            }
        }
    }
    return true;
}
