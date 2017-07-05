/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 07/04/2017 19:57:24
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

ServerMap::ServerPathFinder::ServerPathFinder(ServerMap *pMap, int nMaxStep, bool bCheckCreature)
    : AStarPathFinder(
            // 1. step check function
            //    validate pMap in constructor body
            [pMap, nMaxStep](int nSrcX, int nSrcY, int nDstX, int nDstY) -> bool
            {
                // for server we always check ground
                // there shouldn't be any motion through invalid grids while client could be

                // skip some parameter checking
                // but put the code here for completion

                if(0){
                    if(true
                            && nMaxStep != 1
                            && nMaxStep != 2
                            && nMaxStep != 3){

                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", nMaxStep);
                        return false;
                    }

                    int nDistance2 = LDistance2(nSrcX, nSrcY, nDstX, nDstY);
                    if(true
                            && nDistance2 != 1
                            && nDistance2 != 2
                            && nDistance2 != nMaxStep * nMaxStep
                            && nDistance2 != nMaxStep * nMaxStep * 2){

                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid step checked: (%d, %d) -> (%d, %d)", nSrcX, nSrcY, nDstX, nDstY);
                        return false;
                    }
                }

                // now we are sure:
                // 1. nMaxStep is 1, 2, 3
                // 2. distance of given src -> dst is 1, 2, 3

                // no matter bCheckCreature set or not
                // we allow any hops if the ground is valid, ignoring the creature here

                return pMap->CanMove(false, nSrcX, nSrcY, nDstX, nDstY);
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
            //    
            //    also read comments in mir2x/client/src/clientpathfinder.cpp
            //    there are more additional issues and solutions for the cost assignment

            [pMap, nMaxStep, bCheckCreature](int nSrcX, int nSrcY, int nDstX, int nDstY) -> double
            {
                // checking parameter here
                // skipped since it's done in AStarPathFinder

                if(0){
                    if(true
                            && nMaxStep != 1
                            && nMaxStep != 2
                            && nMaxStep != 3){

                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", nMaxStep);
                        return 10000.00;
                    }

                    int nDistance2 = LDistance2(nSrcX, nSrcY, nDstX, nDstY);
                    if(true
                            && nDistance2 != 1
                            && nDistance2 != 2
                            && nDistance2 != nMaxStep * nMaxStep
                            && nDistance2 != nMaxStep * nMaxStep * 2){

                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid step checked: (%d, %d) -> (%d, %d)", nSrcX, nSrcY, nDstX, nDstY);
                        return 10000.00;
                    }
                }

                double fExtraPen = 0.00;
                switch(LDistance2(nSrcX, nSrcY, nDstX, nDstY)){
                    case  1: fExtraPen = 0.00 + 0.01; break;
                    case  2: fExtraPen = 0.10 + 0.01; break;
                    case  4: fExtraPen = 0.00 + 0.02; break;
                    case  8: fExtraPen = 0.10 + 0.02; break;
                    case  9: fExtraPen = 0.00 + 0.03; break;
                    case 18: fExtraPen = 0.10 + 0.03; break;
                    default:
                        {
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid step checked: (%d, %d) -> (%d, %d)", nSrcX, nSrcY, nDstX, nDstY);
                            return 10000.00;
                        }
                }

                // for server any hops provided to this function has passed the ground checking
                // this is done in the above lambda through ServerMap::CanMove(false, srcLoc, dstLoc)

                // here we need to assign different value for provided hops
                // since the hops provided to this function doesn't do creature check
                // we assign higher cost if bCheckCreature is set and ServerMap::CanMove(true, srcLoc, dstLoc) fails

                if(bCheckCreature){

                    // if there is no co on the way we take it
                    // however if there is, we can still take it but with very high cost
                    // for example: now A is targeting at C
                    //    +---+---+---+---+---+
                    //    |   |   |   | C |   |
                    //    +---+---+---+---+---+
                    //    |   |   | B |   |   |
                    //    +---+---+---+---+---+
                    //    |   | A |   |   |   |
                    //    +---+---+---+---+---+
                    //    |   |   |   |   |   |
                    //    +---+---+---+---+---+
                    // then we try to find a path Loc(A) -> Loc(C)
                    // if 1. we always allow if GroundValid(LOC) is true : co ignored
                    //    2. we always allow if CanMove(LOC)     is true : co affects
                    //
                    // for solution-1: A will stop at current position since A try to move to LOC(B) but it's occupied
                    //     solution-2: LOC(A) -> LOC(C) is invalid since start and end point has been taken by themselves
                    //
                    // so my solution: 1. always allow if GroundValid(LOC) is true
                    //                 2. if currently there is CO in the cell we give a higher cost
                    // then A will bypass B to go to C and stop as close as possible to C
                    //
                    // this also saves if C is surrendered by a lot of CO's
                    // in which case solution-1 will make A stop at a far distance between C
                    //               solution-2 will make C un-reachable
                    //            my solution   will make A stop at a place as close as possible to C

                    return (pMap->CanMove(true, nSrcX, nSrcY, nDstX, nDstY) ? 1.00 : 100.00) + fExtraPen;
                }else{
                    // won't check creature
                    // then all walk-able step get cost 1.0 + delta
                    return 1.00 + fExtraPen;
                }
            },

            // max step length for each hop, valid step size : 1, 2, 3
            // check value in AStarPathFinder::AStarPathFinder() and ServerPathFinder::ServerPathFinder()
            nMaxStep
      )
{
    // we do it here to complete the logic
    // this also will be checked in AStarPathFinder::AStarPathFinder()
    switch(nMaxStep){
        case 1:
        case 2:
        case 3:
            {
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", nMaxStep);
                break;
            }
    }

    if(!pMap){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid argument: ServerMap = %p, CheckCreature = %d", pMap, (int)(bCheckCreature));
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
        case MPK_TRYLEAVE:
            {
                On_MPK_TRYLEAVE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_UPDATEHP:
            {
                On_MPK_UPDATEHP(rstMPK, rstFromAddr);
                break;
            }
        case MPK_DEADFADEOUT:
            {
                On_MPK_DEADFADEOUT(rstMPK, rstFromAddr);
                break;
            }
        case MPK_ACTION:
            {
                On_MPK_ACTION(rstMPK, rstFromAddr);
                break;
            }
        case MPK_BADACTORPOD:
            {
                On_MPK_BADACTORPOD(rstMPK, rstFromAddr);
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
        case MPK_QUERYCORECORD:
            {
                On_MPK_QUERYCORECORD(rstMPK, rstFromAddr);
                break;
            }
        case MPK_QUERYCOCOUNT:
            {
                On_MPK_QUERYCOCOUNT(rstMPK, rstFromAddr);
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_FATAL, "Unsupported message: %s", rstMPK.Name());
                break;
            }
    }
}

bool ServerMap::CanMove(bool bCheckCreature, int nX, int nY)
{
    if(true
            && (m_Mir2xMapData.Valid())
            && (m_Mir2xMapData.ValidC(nX, nY))
            && (m_Mir2xMapData.Cell(nX, nY).Param & 0X80000000)
            && (m_Mir2xMapData.Cell(nX, nY).Param & 0X00800000)){

        if(bCheckCreature){
            for(auto nUID: m_UIDRecordV2D[nX][nY]){
                extern MonoServer *g_MonoServer;
                if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                    if(stUIDRecord.ClassFrom<CharObject>()){
                        return false;
                    }
                }
            }
        }
        return true;
    }
    return false;
}

bool ServerMap::CanMove(bool bCheckCreature, int nX0, int nY0, int nX1, int nY1)
{
    int nMaxIndex = -1;
    switch(LDistance2(nX0, nY0, nX1, nY1)){
        case 0:
            {
                nMaxIndex = 0;
                break;
            }
        case 1:
        case 2:
            {
                nMaxIndex = 1;
                break;
            }
        case 4:
        case 8:
            {
                nMaxIndex = 2;
                break;
            }
        case  9:
        case 18:
            {
                nMaxIndex = 3;
                break;
            }
        default:
            {
                return false;
            }
    }

    int nDX = (nX1 > nX0) - (nX1 < nX0);
    int nDY = (nY1 > nY0) - (nY1 < nY0);

    for(int nIndex = 0; nIndex <= nMaxIndex; ++nIndex){
        if(!CanMove(bCheckCreature, nX0 + nDX * nIndex, nY0 + nDY * nIndex)){
            return false;
        }
    }
    return true;
}

bool ServerMap::RandomLocation(int *pX, int *pY)
{
    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){
            if(CanMove(false, nX, nY)){
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
            if(CanMove(false, nX, nY)){
                if(m_CellRecordV2D[nX][nY].Lock){
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

Theron::Address ServerMap::Activate()
{
    auto stAddress = ActiveObject::Activate();

    delete m_Metronome;
    m_Metronome = new Metronome(300);
    m_Metronome->Activate(GetAddress());

    return stAddress;
}
