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

ServerMap::ServerPathFinder::ServerPathFinder(const ServerMap *pMap, int nMaxStep, int nCheckCO)
    : AStarPathFinder([this](int nSrcX, int nSrcY, int nDstX, int nDstY) -> double
      {
          if(0){
              if(true
                      && MaxStep() != 1
                      && MaxStep() != 2
                      && MaxStep() != 3){

                  extern MonoServer *g_MonoServer;
                  g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", MaxStep());
                  return 10000.00;
              }

              int nDistance2 = LDistance2(nSrcX, nSrcY, nDstX, nDstY);
              if(true
                      && nDistance2 != 1
                      && nDistance2 != 2
                      && nDistance2 != MaxStep() * MaxStep()
                      && nDistance2 != MaxStep() * MaxStep() * 2){

                  extern MonoServer *g_MonoServer;
                  g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid step checked: (%d, %d) -> (%d, %d)", nSrcX, nSrcY, nDstX, nDstY);
                  return 10000.00;
              }
          }

          const int nCheckLock = m_CheckCO;
          return m_Map->OneStepCost(m_CheckCO, nCheckLock, nSrcX, nSrcY, nDstX, nDstY);
      }, nMaxStep)
    , m_Map(pMap)
    , m_CheckCO(nCheckCO)
{
    if(!pMap){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid argument: ServerMap = %p, CheckCreature = %d", pMap, nCheckCO);
    }

    switch(nCheckCO){
        case 0:
        case 1:
        case 2:
            {
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid CheckCO provided: %d, should be (0, 1, 2)", nCheckCO);
                break;
            }
    }

    switch(MaxStep()){
        case 1:
        case 2:
        case 3:
            {
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", MaxStep());
                break;
            }
    }
}

ServerMap::ServerMap(ServiceCore *pServiceCore, uint32_t nMapID)
    : ServerObject(UIDFunc::GetMapUID(nMapID))
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
    , m_CellVec2D()
    , m_LuaModule(nullptr)
{
    m_CellVec2D.clear();
    if(m_Mir2xMapData.Valid()){
        m_CellVec2D.resize(W());
        for(auto &rstStateLine: m_CellVec2D){
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
                    GetCell(stLinkEntry.X + nW,stLinkEntry.Y + nH).MapID   = DBCOM_MAPID(stLinkEntry.EndName);
                    GetCell(stLinkEntry.X + nW,stLinkEntry.Y + nH).SwitchX = stLinkEntry.EndX;
                    GetCell(stLinkEntry.X + nW,stLinkEntry.Y + nH).SwitchY = stLinkEntry.EndY;
                }
            }
        }else{
            break;
        }
    }
}

void ServerMap::OperateAM(const MessagePack &rstMPK)
{
    switch(rstMPK.Type()){
        case MPK_PICKUP:
            {
                On_MPK_PICKUP(rstMPK);
                break;
            }
        case MPK_NEWDROPITEM:
            {
                On_MPK_NEWDROPITEM(rstMPK);
                break;
            }
        case MPK_TRYLEAVE:
            {
                On_MPK_TRYLEAVE(rstMPK);
                break;
            }
        case MPK_UPDATEHP:
            {
                On_MPK_UPDATEHP(rstMPK);
                break;
            }
        case MPK_DEADFADEOUT:
            {
                On_MPK_DEADFADEOUT(rstMPK);
                break;
            }
        case MPK_ACTION:
            {
                On_MPK_ACTION(rstMPK);
                break;
            }
        case MPK_BADACTORPOD:
            {
                On_MPK_BADACTORPOD(rstMPK);
                break;
            }
        case MPK_TRYMOVE:
            {
                On_MPK_TRYMOVE(rstMPK);
                break;
            }
        case MPK_PATHFIND:
            {
                On_MPK_PATHFIND(rstMPK);
                break;
            }
        case MPK_TRYMAPSWITCH:
            {
                On_MPK_TRYMAPSWITCH(rstMPK);
                break;
            }
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK);
                break;
            }
        case MPK_TRYSPACEMOVE:
            {
                On_MPK_TRYSPACEMOVE(rstMPK);
                break;
            }
        case MPK_ADDCHAROBJECT:
            {
                On_MPK_ADDCHAROBJECT(rstMPK);
                break;
            }
        case MPK_PULLCOINFO:
            {
                On_MPK_PULLCOINFO(rstMPK);
                break;
            }
        case MPK_QUERYCOCOUNT:
            {
                On_MPK_QUERYCOCOUNT(rstMPK);
                break;
            }
        case MPK_QUERYRECTUIDLIST:
            {
                On_MPK_QUERYRECTUIDLIST(rstMPK);
                break;
            }
        case MPK_OFFLINE:
            {
                On_MPK_OFFLINE(rstMPK);
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
            for(auto nUID: GetUIDList(nX, nY)){
                if(auto nType = UIDFunc::GetUIDType(nUID); nType == UID_PLY || nType == UID_MON){
                    return false;
                }
            }
        }

        if(bCheckLock){
            if(GetCell(nX, nY).Locked){
                return false;
            }
        }
        return true;
    }
    return false;
}

double ServerMap::OneStepCost(int nCheckCO, int nCheckLock, int nX0, int nY0, int nX1, int nY1) const
{
    switch(nCheckCO){
        case 0:
        case 1:
        case 2:
            {
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid CheckCO provided: %d, should be (0, 1, 2)", nCheckCO);
                return -1.00;
            }
    }

    switch(nCheckLock){
        case 0:
        case 1:
        case 2:
            {
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid CheckLock provided: %d, should be (0, 1, 2)", nCheckLock);
                return -1.00;
            }
    }

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
                return -1.00;
            }
    }

    int nDX = (nX1 > nX0) - (nX1 < nX0);
    int nDY = (nY1 > nY0) - (nY1 < nY0);

    double fExtraPen = 0.00;
    for(int nIndex = 0; nIndex <= nMaxIndex; ++nIndex){
        switch(auto nGrid = CheckPathGrid(nX0 + nDX * nIndex, nY0 + nDY * nIndex)){
            case PathFind::FREE:
                {
                    break;
                }
            case PathFind::OCCUPIED:
                {
                    switch(nCheckCO){
                        case 1:
                            {
                                fExtraPen += 100.00;
                                break;
                            }
                        case 2:
                            {
                                return -1.00;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    break;
                }
            case PathFind::LOCKED:
                {
                    if(((nIndex == 0) || (nIndex == nMaxIndex))){
                        switch(nCheckLock){
                            case 1:
                                {
                                    fExtraPen += 100.00;
                                    break;
                                }
                            case 2:
                                {
                                    return -1.00;
                                }
                            default:
                                {
                                    break;
                                }
                        }
                    }
                    break;
                }
            case PathFind::INVALID:
            case PathFind::OBSTACLE:
                {
                    return -1.00;
                }
            default:
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid grid provided: %d at (%d, %d)", nGrid, nX0 + nDX * nIndex, nY0 + nDY * nIndex);
                    break;
                }
        }
    }

    return 1.00 + nMaxIndex * 0.10 + fExtraPen;
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

uint64_t ServerMap::Activate()
{
    if(auto nUID = ServerObject::Activate()){
        delete m_LuaModule;
        m_LuaModule = new ServerMap::ServerMapLuaModule();
        RegisterLuaExport(m_LuaModule);
        return nUID;
    }
    return 0;
}

void ServerMap::AddGridUID(uint64_t nUID, int nX, int nY)
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

void ServerMap::RemoveGridUID(uint64_t nUID, int nX, int nY)
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

bool ServerMap::DoUIDList(int nX, int nY, const std::function<bool(uint64_t)> &fnOP)
{
    if(!ValidC(nX, nY)){
        return false;
    }

    if(!fnOP){
        return false;
    }

    for(auto nUID: GetUIDList(nX, nY)){
        if(fnOP(nUID)){
            return true;
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
                for(auto nUID: GetUIDList(nX, nY)){
                    if(UIDFunc::GetUIDType(nUID) == UID_PLY){
                        m_ActorPod->Forward(nUID, {MPK_SHOWDROPITEM, stAMSDI});
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
    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){
            for(auto nUID: GetUIDList(nX, nY)){
                if(UIDFunc::GetUIDType(nUID) == UID_MON){
                    if(nMonsterID){
                        nCount += ((UIDFunc::GetMonsterID(nUID) == nMonsterID) ? 1 : 0);
                    }else{
                        nCount++;
                    }
                }
            }
        }
    }
    return nCount;
}

void ServerMap::NotifyNewCO(uint64_t nUID, int nX, int nY)
{
    AMNotifyNewCO stAMNNCO;
    std::memset(&stAMNNCO, 0, sizeof(stAMNNCO));

    stAMNNCO.UID = nUID;
    DoCircle(nX, nY, 20, [this, stAMNNCO](int nX, int nY) -> bool
    {
        if(true || ValidC(nX, nY)){
            DoUIDList(nX, nY, [this, stAMNNCO](uint64_t nUID)
            {
                if(nUID != stAMNNCO.UID){
                    m_ActorPod->Forward(nUID, {MPK_NOTIFYNEWCO, stAMNNCO});
                }
                return false;
            });
        }
        return false;
    });
}

Monster *ServerMap::AddMonster(uint32_t nMonsterID, uint64_t nMasterUID, int nX, int nY, bool bRandom)
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

int ServerMap::CheckPathGrid(int nX, int nY) const
{
    if(!m_Mir2xMapData.ValidC(nX, nY)){
        return PathFind::INVALID;
    }

    if(!m_Mir2xMapData.Cell(nX, nY).CanThrough()){
        return PathFind::OBSTACLE;
    }

    // for(auto nUID: GetUIDList(nX, nY)){
    //     if(auto nType = UIDFunc::GetUIDType(nUID); nType == UID_PLY || nType == UID_MON){
    //         return PatFind::OCCUPIED;
    //     }
    // }

    if(!GetUIDList(nX, nY).empty()){
        return PathFind::OCCUPIED;
    }

    if(GetCell(nX, nY).Locked){
        return PathFind::LOCKED;
    }

    return PathFind::FREE;
}
