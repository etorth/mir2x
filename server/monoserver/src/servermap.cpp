/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
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

#include <sstream>
#include <fstream>
#include <algorithm>
#include "player.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "actorpod.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "condcheck.hpp"
#include "servermap.hpp"
#include "mapbindbn.hpp"
#include "charobject.hpp"
#include "monoserver.hpp"
#include "dbcomrecord.hpp"
#include "rotatecoord.hpp"
#include "serverconfigurewindow.hpp"

ServerMap::ServerMapLuaModule::ServerMapLuaModule()
    : BatchLuaModule()
{}

ServerMap::ServerPathFinder::ServerPathFinder(ServerMap *pMap, int nMaxStep, bool bCheckCO)
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

                // no matter bCheckCO set or not
                // we allow all grids if the ground is valid, ignoring the creature and lock

                return pMap->CanMove(false, false, nSrcX, nSrcY, nDstX, nDstY);
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

            [pMap, nMaxStep, bCheckCO](int nSrcX, int nSrcY, int nDstX, int nDstY) -> double
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
                // we assign higher cost if bCheckCO is set and ServerMap::CanMove(true, true, srcLoc, dstLoc) fails

                if(bCheckCO){

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
                    // if 1. we always allow if CanMove(0, 0, LOC) is true : co ignored
                    //    2. we always allow if CanMove(1, 1, LOC) is true : co affects
                    //
                    // for solution-1: A will stop at current position since A try to move to LOC(B) but it's occupied
                    //     solution-2: LOC(A) -> LOC(C) is invalid since start and end point has been taken by themselves
                    //
                    // so my solution: 1. always allow if CanMove(0, 0, LOC) is true
                    //                 2. if currently there is CO in the cell we give a higher cost
                    // then A will bypass B to go to C and stop as close as possible to C
                    //
                    // this also saves if C is surrendered by a lot of CO's
                    // in which case solution-1 will make A stop at a far distance between C
                    //               solution-2 will make C un-reachable
                    //            my solution   will make A stop at a place as close as possible to C

                    // new issue found for current cost strategy
                    //
                    //             O O D O O
                    //             O O X O O
                    //             B O A O C
                    //             O O O O O
                    //
                    // if now we want A->D with step size = 2, we can do (A->B->D) or (A->C->D), the
                    // step (A->B) and (A->C) are expensive since A is occupied by the path-finding
                    // requestor. But now if between A and C the grid is occupied by another CO, our
                    // current algorithm can't prefer (A->B->D) instead of (A->C->D) !!!

                    // reason is we only get yes/no anwser for (A->B) and (A->D) but no detailed
                    // information to estimate a cost. solution: assign cost inside map rather than
                    // in the path finder, which need me to introduce new function
                    //
                    //   doulbe ServerMap::MoveCost(bool, bool, bool, int, int, int, int)
                    //   
                    // this function gives cost of single hop
                    // in future's version fExtraPen should be penalty for turn
                    // and this function can enable we put dynamical cost for some busy hops

                    return pMap->MoveCost(true, true, nSrcX, nSrcY, nDstX, nDstY) + fExtraPen;
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
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid argument: ServerMap = %p, CheckCreature = %d", pMap, (int)(bCheckCO));
    }
}

ServerMap::ServerMap(ServiceCore *pServiceCore, uint32_t nMapID)
    : ActiveObject()
    , m_ID(nMapID)
    , m_Mir2xMapData(*([nMapID]() -> Mir2xMapData *
      {
          // server is multi-thread
          // but creating server map is always in service core

          extern MapBinDBN *g_MapBinDBN;
          auto pMir2xMapData = g_MapBinDBN->Retrieve(nMapID);

          // when constructing a servermap
          // servicecore should test if current nMapID valid
          condcheck(pMir2xMapData);
          return pMir2xMapData;
      }()))
    , m_ServiceCore(pServiceCore)
    , m_CellRecordV2D()
    , m_LuaModule(nullptr)
{
    m_CellRecordV2D.clear();
    if(m_Mir2xMapData.Valid()){
        m_CellRecordV2D.resize(W());
        for(auto &rstStateLine: m_CellRecordV2D){
            rstStateLine.resize(H());
        }
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Load map failed: ID = %d, Name = %s", nMapID, DBCOM_MAPRECORD(nMapID).Name);
        g_MonoServer->Restart();
    }

    for(auto stLinkEntry: DBCOM_MAPRECORD(nMapID).LinkArray){
        if(true
                && stLinkEntry.W > 0
                && stLinkEntry.H > 0
                && ValidC(stLinkEntry.X, stLinkEntry.Y)){

            for(int nW = 0; nW < stLinkEntry.W; ++nW){
                for(int nH = 0; nH < stLinkEntry.H; ++nH){
                    m_CellRecordV2D[stLinkEntry.X + nW][stLinkEntry.Y + nH].UID     = 0;
                    m_CellRecordV2D[stLinkEntry.X + nW][stLinkEntry.Y + nH].MapID   = DBCOM_MAPID(stLinkEntry.EndName);
                    m_CellRecordV2D[stLinkEntry.X + nW][stLinkEntry.Y + nH].SwitchX = stLinkEntry.EndX;
                    m_CellRecordV2D[stLinkEntry.X + nW][stLinkEntry.Y + nH].SwitchY = stLinkEntry.EndY;
                    m_CellRecordV2D[stLinkEntry.X + nW][stLinkEntry.Y + nH].Query   = QUERY_NONE;
                }
            }
        }else{
            break;
        }
    }
}

void ServerMap::OperateAM(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    switch(rstMPK.Type()){
        case MPK_PICKUP:
            {
                On_MPK_PICKUP(rstMPK, rstFromAddr);
                break;
            }
        case MPK_NEWDROPITEM:
            {
                On_MPK_NEWDROPITEM(rstMPK, rstFromAddr);
                break;
            }
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
        case MPK_QUERYCOCOUNT:
            {
                On_MPK_QUERYCOCOUNT(rstMPK, rstFromAddr);
                break;
            }
        case MPK_QUERYRECTUIDLIST:
            {
                On_MPK_QUERYRECTUIDLIST(rstMPK, rstFromAddr);
                break;
            }
        case MPK_OFFLINE:
            {
                On_MPK_OFFLINE(rstMPK, rstFromAddr);
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

bool ServerMap::GroundValid(int nX, int nY) const
{
    return true
        && m_Mir2xMapData.Valid()
        && m_Mir2xMapData.ValidC(nX, nY)
        && m_Mir2xMapData.Cell(nX, nY).CanThrough();
}

bool ServerMap::CanMove(bool bCheckCO, bool bCheckLock, int nX, int nY)
{
    if(GroundValid(nX, nY)){
        if(bCheckCO){
            for(auto nUID: m_CellRecordV2D[nX][nY].UIDList){
                extern MonoServer *g_MonoServer;
                if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                    if(false
                            || stUIDRecord.ClassFrom<Player >()
                            || stUIDRecord.ClassFrom<Monster>()){
                        return false;
                    }
                }
            }
        }

        if(bCheckLock){
            if(m_CellRecordV2D[nX][nY].Lock){
                return false;
            }
        }
        return true;
    }
    return false;
}

bool ServerMap::CanMove(bool bCheckCO, bool bCheckLock, int nX0, int nY0, int nX1, int nY1)
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
        bool bCheckCurrLock = false;
        if(true
                && bCheckLock
                && ((nIndex == 0) || (nIndex == nMaxIndex))){
            bCheckCurrLock = true;
        }

        if(!CanMove(bCheckCO, bCheckCurrLock, nX0 + nDX * nIndex, nY0 + nDY * nIndex)){
            return false;
        }
    }
    return true;
}

double ServerMap::MoveCost(bool bCheckCO, bool bCheckLock, int nX0, int nY0, int nX1, int nY1)
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
                return 10000.00;
            }
    }

    int nDX = (nX1 > nX0) - (nX1 < nX0);
    int nDY = (nY1 > nY0) - (nY1 < nY0);

    double fMoveCost = 0.0;
    for(int nIndex = 0; nIndex <= nMaxIndex; ++nIndex){
        auto nCurrX = nX0 + nDX * nIndex;
        auto nCurrY = nY0 + nDY * nIndex;

        // validate current grid first
        // for server's side any motion needs to be inside valid grids

        if(GroundValid(nCurrX, nCurrY)){

            // OK current grid is capable to contain co's
            // check the co's and locks

            bool bCheckCurrLock = false;
            if(true
                    && bCheckLock
                    && ((nIndex == 0) || (nIndex == nMaxIndex))){
                bCheckCurrLock = true;
            }

            fMoveCost += (CanMove(bCheckCO, bCheckCurrLock, nCurrX, nCurrY) ? 1.00 : 100.00);

        }else{

            // current grid is not valid
            // ignore alculated cost and immediately return infinite
            return 10000.00;
        }
    }

    return fMoveCost;
}

bool ServerMap::GetValidGrid(int *pX, int *pY, bool bRandom)
{
    if(pX && pY){

        auto nX = *pX;
        auto nY = *pY;

        bool bValidLoc = true;
        if(false
                || !In(ID(), nX, nY)
                || !CanMove(true, true, nX, nY)){

            // have to check if we can do random pick
            // the location field provides an invalid location
            bValidLoc = false;

            if(bRandom){
                if(In(ID(), nX, nY)){
                    // OK we failed to add monster at the specified location
                    // but still to try to add near it
                }else{
                    // randomly pick one
                    // an invalid location provided
                    nX = std::rand() % W();
                    nY = std::rand() % H();
                }

                RotateCoord stRC;
                if(stRC.Reset(nX, nY, 0, 0, W(), H())){
                    do{
                        if(true
                                && In(ID(), stRC.X(), stRC.Y())
                                && CanMove(true, true, stRC.X(), stRC.Y())){

                            // find a valid location
                            // use it to add new charobject
                            bValidLoc = true;

                            nX = stRC.X();
                            nY = stRC.Y();
                            break;
                        }
                    }while(stRC.Forward());
                }
            }
        }

        // if we get the valid location, output it
        // otherwise we keep it un-touched

        if(bValidLoc){

            *pX = nX;
            *pY = nY;
        }

        return bValidLoc;
    }
    return false;
}

bool ServerMap::RandomLocation(int *pX, int *pY)
{
    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){
            if(CanMove(false, false, nX, nY)){
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
            if(CanMove(true, true, nX, nY)){
                return false;
            }
        }
    }
    return true;
}

Theron::Address ServerMap::Activate()
{
    auto stAddress = ActiveObject::Activate();

    delete m_LuaModule;
    m_LuaModule = new ServerMap::ServerMapLuaModule();
    RegisterLuaExport(m_LuaModule);

    AddTick();
    return stAddress;
}

void ServerMap::AddGridUID(uint32_t nUID, int nX, int nY)
{
    if(true
            && ValidC(nX, nY)
            && GroundValid(nX, nY)){

        auto &rstUIDList = GetUIDList(nX, nY);
        if(std::find(rstUIDList.begin(), rstUIDList.end(), nUID) == rstUIDList.end()){
            rstUIDList.push_back(nUID);
        }
    }
}

void ServerMap::RemoveGridUID(uint32_t nUID, int nX, int nY)
{
    if(true
            && ValidC(nX, nY)
            && GroundValid(nX, nY)){

        auto &rstUIDList = GetUIDList(nX, nY);
        auto pUIDRecord  = std::find(rstUIDList.begin(), rstUIDList.end(), nUID);

        if(pUIDRecord != rstUIDList.end()){
            std::swap(rstUIDList.back(), *pUIDRecord);
            rstUIDList.pop_back();
        }
    }
}

bool ServerMap::DoUIDList(int nX, int nY, const std::function<bool(const UIDRecord &)> &fnOP)
{
    if(ValidC(nX, nY)){
        auto &rstUIDList = GetUIDList(nX, nY);
        for(size_t nIndex = 0; nIndex < rstUIDList.size();){
            extern MonoServer *g_MonoServer;
            if(auto stUIDRecord = g_MonoServer->GetUIDRecord(rstUIDList[nIndex])){
                if(!fnOP){
                    return false;
                }

                if(fnOP(stUIDRecord)){
                    return true;
                }
                nIndex++;
            }else{
                rstUIDList[nIndex] = rstUIDList.back();
                rstUIDList.pop_back();
            }
        }
    }
    return false;
}

bool ServerMap::DoCircle(int nCX0, int nCY0, int nCR, const std::function<bool(int, int)> &fnOP)
{
    int nW = 2 * nCR - 1;
    int nH = 2 * nCR - 1;

    int nX0 = nCX0 - nCR + 1;
    int nY0 = nCY0 - nCR + 1;

    if(true
            && nW > 0
            && nH > 0
            && RectangleOverlapRegion(0, 0, W(), H(), &nX0, &nY0, &nW, &nH)){

        // get the clip region over the map
        // if no valid region we won't do the rest

        for(int nX = nX0; nX < nX0 + nW; ++nX){
            for(int nY = nY0; nY < nY0 + nH; ++nY){
                if(true || ValidC(nX, nY)){
                    if(LDistance2(nX, nY, nCX0, nCY0) <= (nCR - 1) * (nCR - 1)){
                        if(!fnOP){
                            return false;
                        }

                        if(fnOP(nX, nY)){
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool ServerMap::DoSquare(int nX0, int nY0, int nW, int nH, const std::function<bool(int, int)> &fnOP)
{
    if(true
            && nW > 0
            && nH > 0
            && RectangleOverlapRegion(0, 0, W(), H(), &nX0, &nY0, &nW, &nH)){

        // get the clip region over the map
        // if no valid region we won't do the rest

        for(int nX = nX0; nX < nX0 + nW; ++nX){
            for(int nY = nY0; nY < nY0 + nH; ++nY){
                if(true || ValidC(nX, nY)){
                    if(!fnOP){
                        return false;
                    }

                    if(fnOP(nX, nY)){
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool ServerMap::DoCenterCircle(int nCX0, int nCY0, int nCR, bool bPriority, const std::function<bool(int, int)> &fnOP)
{
    if(!bPriority){
        return DoCircle(nCX0, nCY0, nCR, fnOP);
    }

    int nW = 2 * nCR - 1;
    int nH = 2 * nCR - 1;

    int nX0 = nCX0 - nCR + 1;
    int nY0 = nCY0 - nCR + 1;

    if(true
            && nW > 0
            && nH > 0
            && RectangleOverlapRegion(0, 0, W(), H(), &nX0, &nY0, &nW, &nH)){

        // get the clip region over the map
        // if no valid region we won't do the rest

        RotateCoord stRC;
        if(stRC.Reset(nCX0, nCY0, nX0, nY0, nW, nH)){
            do{
                int nX = stRC.X();
                int nY = stRC.Y();

                if(true || ValidC(nX, nY)){
                    if(LDistance2(nX, nY, nCX0, nCY0) <= (nCR - 1) * (nCR - 1)){
                        if(!fnOP){
                            return false;
                        }

                        if(fnOP(nX, nY)){
                            return true;
                        }
                    }
                }
            }while(stRC.Forward());
        }
    }
    return false;
}

bool ServerMap::DoCenterSquare(int nCX, int nCY, int nW, int nH, bool bPriority, const std::function<bool(int, int)> &fnOP)
{
    if(!bPriority){
        return DoSquare(nCX - nW / 2, nCY - nH / 2, nW, nH, fnOP);
    }

    int nX0 = nCX - nW / 2;
    int nY0 = nCY - nH / 2;

    if(true
            && nW > 0
            && nH > 0
            && RectangleOverlapRegion(0, 0, W(), H(), &nX0, &nY0, &nW, &nH)){

        // get the clip region over the map
        // if no valid region we won't do the rest

        RotateCoord stRC;
        if(stRC.Reset(nCX, nCY, nX0, nY0, nW, nH)){
            do{
                int nX = stRC.X();
                int nY = stRC.Y();

                if(true || ValidC(nX, nY)){
                    if(!fnOP){
                        return false;
                    }

                    if(fnOP(nX, nY)){
                        return true;
                    }
                }
            }while(stRC.Forward());
        }
    }
    return false;
}

int ServerMap::FindGroundItem(const CommonItem &rstCommonItem, int nX, int nY)
{
    if(ValidC(nX, nY)){
        auto &rstGroundItemList = GetGroundItemList(nX, nY);
        for(size_t nIndex = 0; nIndex < rstGroundItemList.Length(); ++nIndex){
            if(rstGroundItemList[nIndex] == rstCommonItem){
                return (int)(nIndex);
            }
        }
    }
    return -1;
}

int ServerMap::GroundItemCount(const CommonItem &rstCommonItem, int nX, int nY)
{
    if(ValidC(nX, nY)){
        auto &rstGroundItemList = GetGroundItemList(nX, nY);
        int nCount = 0;
        for(size_t nIndex = 0; nIndex < rstGroundItemList.Length(); ++nIndex){
            if(rstGroundItemList[nIndex] == rstCommonItem){
                nCount++;
            }
        }
        return nCount;
    }
    return -1;
}

void ServerMap::RemoveGroundItem(const CommonItem &rstCommonItem, int nX, int nY)
{
    auto nFind = FindGroundItem(rstCommonItem, nX, nY);
    if(nFind >= 0){
        auto &rstGroundItemList = GetGroundItemList(nX, nY);
        for(int nIndex = nFind; nIndex < ((int)(rstGroundItemList.Length()) - 1); ++nIndex){
            rstGroundItemList[nIndex] = rstGroundItemList[nIndex + 1];
        }
        rstGroundItemList.PopBack();
    }
}

bool ServerMap::AddGroundItem(const CommonItem &rstCommonItem, int nX, int nY)
{
    if(true
            && rstCommonItem
            && GroundValid(nX, nY)){

        // check if item is valid
        // then push back and report, would override if already full

        auto &rstGroundItemList = GetGroundItemList(nX, nY);
        rstGroundItemList.PushBack(rstCommonItem);

        AMShowDropItem stAMSDI;
        std::memset(&stAMSDI, 0, sizeof(stAMSDI));

        stAMSDI.X = nX;
        stAMSDI.Y = nY;

        size_t nCurrLoc = 0;
        for(size_t nIndex = 0; nIndex < rstGroundItemList.Length(); ++nIndex){
            if(rstGroundItemList[nIndex]){
                if(nCurrLoc < std::extent<decltype(stAMSDI.IDList)>::value){
                    stAMSDI.IDList[nCurrLoc].ID   = rstGroundItemList[nIndex].ID();
                    stAMSDI.IDList[nCurrLoc].DBID = rstGroundItemList[nIndex].DBID();
                    nCurrLoc++;
                }else{
                    break;
                }
            }
        }

        auto fnNotifyDropItem = [this, stAMSDI](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                for(auto nUID: m_CellRecordV2D[nX][nY].UIDList){
                    extern MonoServer *g_MonoServer;
                    if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                        if(stUIDRecord.ClassFrom<Player>()){
                            m_ActorPod->Forward({MPK_SHOWDROPITEM, stAMSDI}, stUIDRecord.GetAddress());
                        }
                    }
                }
            }
            return false;
        };

        DoCircle(nX, nY, 10, fnNotifyDropItem);
        return true;
    }
    return false;
}

int ServerMap::GetMonsterCount(uint32_t nMonsterID)
{
    int nCount = 0;
    for(auto &rstLine: m_CellRecordV2D){
        for(auto &rstRecord: rstLine){
            for(auto nUID: rstRecord.UIDList){
                extern MonoServer *g_MonoServer;
                if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                    if(stUIDRecord.ClassFrom<Monster>()){
                        if(nMonsterID){
                            nCount += ((stUIDRecord.GetInvarData().Monster.MonsterID == nMonsterID) ? 1 : 0);
                        }else{
                            nCount++;
                        }
                    }
                }
            }
        }
    }
    return nCount;
}

void ServerMap::NotifyNewCO(uint32_t nUID, int nX, int nY)
{
    extern MonoServer *g_MonoServer;
    if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){

        AMNotifyNewCO stAMNNCO;
        std::memset(&stAMNNCO, 0, sizeof(stAMNNCO));

        stAMNNCO.UID = nUID;

        auto fnNotifyNewCO = [this, stAMNNCO](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                auto fnDoList = [this, stAMNNCO](const UIDRecord &rstUIDRecord) -> bool
                {
                    if(rstUIDRecord.UID() != stAMNNCO.UID){
                        m_ActorPod->Forward({MPK_NOTIFYNEWCO, stAMNNCO}, rstUIDRecord.GetAddress());
                    }
                    return false;
                };
                DoUIDList(nX, nY, fnDoList);
            }
            return false;
        };
        DoCircle(nX, nY, 20, fnNotifyNewCO);
    }
}

Monster *ServerMap::AddMonster(uint32_t nMonsterID, uint32_t nMasterUID, int nX, int nY, bool bRandom)
{
    if(GetValidGrid(&nX, &nY, bRandom)){
        auto pMonster = new Monster
        {
            nMonsterID,
            m_ServiceCore,
            this,
            nX,
            nY,
            DIR_UP,
            STATE_INCARNATED,
            nMasterUID,
        };

        pMonster->Activate();

        AddGridUID(pMonster->UID(), nX, nY);
        NotifyNewCO(pMonster->UID(), nX, nY);

        return pMonster;
    }
    return nullptr;
}

Player *ServerMap::AddPlayer(uint32_t nDBID, int nX, int nY, int nDirection, bool bRandom)
{
    if(GetValidGrid(&nX, &nY, bRandom)){
        auto pPlayer = new Player
        {
            nDBID,
            m_ServiceCore,
            this,
            nX,
            nY,
            nDirection,
            STATE_INCARNATED,
        };

        pPlayer->Activate();

        AddGridUID(pPlayer->UID(), nX, nY);
        NotifyNewCO(pPlayer->UID(), nX, nY);

        return pPlayer;
    }
    return nullptr;
}

bool ServerMap::RegisterLuaExport(ServerMap::ServerMapLuaModule *pModule)
{
    if(pModule){

        // load lua script to the module
        {
            extern ServerConfigureWindow *g_ServerConfigureWindow;
            auto szScriptPath = g_ServerConfigureWindow->GetScriptPath();
            if(szScriptPath.empty()){
                szScriptPath  = "/home/anhong/mir2x/server/monoserver/script/map";
            }

            std::string szCommandFile = ((szScriptPath + "/") + DBCOM_MAPRECORD(ID()).Name) + ".lua";

            std::stringstream stCommand;
            std::ifstream stCommandFile(szCommandFile.c_str());

            stCommand << stCommandFile.rdbuf();
            pModule->LoadBatch(stCommand.str().c_str());
        }

        // register lua functions/variables related *this* map

        pModule->GetLuaState().set_function("getMapID", [this]() -> int
        {
            return (int)(ID());
        });

        pModule->GetLuaState().set_function("getMapName", [this]() -> std::string
        {
            return std::string(DBCOM_MAPRECORD(ID()).Name);
        });

        pModule->GetLuaState().set_function("getMonsterCount", [this](sol::variadic_args stVariadicArgs) -> int
        {
            std::vector<sol::object> stArgList(stVariadicArgs.begin(), stVariadicArgs.end());
            switch(stArgList.size()){
                case 0:
                    {
                        return GetMonsterCount(0);
                    }
                case 1:
                    {
                        if(stArgList[0].is<int>()){
                            int nMonsterID = stArgList[0].as<int>();
                            if(nMonsterID >= 0){
                                return GetMonsterCount(nMonsterID);
                            }
                        }else if(stArgList[0].is<std::string>()){
                            int nMonsterID = DBCOM_MONSTERID(stArgList[0].as<std::string>().c_str());
                            if(nMonsterID >= 0){
                                return GetMonsterCount(nMonsterID);
                            }
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
            return -1;
        });

        pModule->GetLuaState().set_function("addMonster", [this](sol::object stMonsterID, sol::variadic_args stVariadicArgs) -> bool
        {
            uint32_t nMonsterID = 0;

            if(stMonsterID.is<int>()){
                nMonsterID = stMonsterID.as<int>();
            }else if(stMonsterID.is<std::string>()){
                nMonsterID = DBCOM_MONSTERID(stMonsterID.as<std::string>().c_str());
            }else{
                return false;
            }

            std::vector<sol::object> stArgList(stVariadicArgs.begin(), stVariadicArgs.end());
            switch(stArgList.size()){
                case 0:
                    {
                        return AddMonster(nMonsterID, 0, -1, -1, true);
                    }
                case 2:
                    {
                        if(true
                                && stArgList[0].is<int>()
                                && stArgList[1].is<int>()){

                            auto nX = stArgList[0].as<int>();
                            auto nY = stArgList[1].as<int>();

                            return AddMonster(nMonsterID, 0, nX, nY, true);
                        }
                        break;
                    }
                case 3:
                    {
                        if(true
                                && stArgList[0].is<int >()
                                && stArgList[1].is<int >()
                                && stArgList[2].is<bool>()){

                            auto nX      = stArgList[0].as<int >();
                            auto nY      = stArgList[1].as<int >();
                            auto bRandom = stArgList[2].as<bool>();

                            return AddMonster(nMonsterID, 0, nX, nY, bRandom);
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }

            return false;
        });
        return true;
    }
    return false;
}
