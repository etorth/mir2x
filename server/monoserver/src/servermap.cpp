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
#include "uidf.hpp"
#include "npchar.hpp"
#include "player.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "actorpod.hpp"
#include "mathf.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "mapbindb.hpp"
#include "condcheck.hpp"
#include "servermap.hpp"
#include "charobject.hpp"
#include "monoserver.hpp"
#include "dbcomrecord.hpp"
#include "rotatecoord.hpp"
#include "serverconfigurewindow.hpp"

extern MapBinDB *g_mapBinDB;
extern MonoServer *g_monoServer;
extern ServerConfigureWindow *g_serverConfigureWindow;

ServerMap::ServerMapLuaModule::ServerMapLuaModule()
    : BatchLuaModule()
{}

ServerMap::ServerPathFinder::ServerPathFinder(const ServerMap *pMap, int nMaxStep, int nCheckCO)
    : AStarPathFinder([this](int nSrcX, int nSrcY, int nDstX, int nDstY) -> double
      {
          // comment the following code out
          // seems it triggers some wired msvc compilation error:
          // error C2248: 'AStarPathFinder::MaxStep': cannot access protected member declared in class 'AStarPathFinder'

          // if(0){
          //     if(true
          //             && MaxStep() != 1
          //             && MaxStep() != 2
          //             && MaxStep() != 3){
          //
          //         g_monoServer->addLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", MaxStep());
          //         return 10000.00;
          //     }
          //
          //     int nDistance2 = mathf::LDistance2(nSrcX, nSrcY, nDstX, nDstY);
          //     if(true
          //             && nDistance2 != 1
          //             && nDistance2 != 2
          //             && nDistance2 != MaxStep() * MaxStep()
          //             && nDistance2 != MaxStep() * MaxStep() * 2){
          //
          //         g_monoServer->addLog(LOGTYPE_FATAL, "Invalid step checked: (%d, %d) -> (%d, %d)", nSrcX, nSrcY, nDstX, nDstY);
          //         return 10000.00;
          //     }
          // }

          const int nCheckLock = m_checkCO;
          return m_map->OneStepCost(m_checkCO, nCheckLock, nSrcX, nSrcY, nDstX, nDstY);
      }, nMaxStep)
    , m_map(pMap)
    , m_checkCO(nCheckCO)
{
    if(!pMap){
        g_monoServer->addLog(LOGTYPE_FATAL, "Invalid argument: ServerMap = %p, CheckCreature = %d", pMap, nCheckCO);
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
                g_monoServer->addLog(LOGTYPE_FATAL, "Invalid CheckCO provided: %d, should be (0, 1, 2)", nCheckCO);
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
                g_monoServer->addLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", MaxStep());
                break;
            }
    }
}

ServerMap::ServerMap(ServiceCore *pServiceCore, uint32_t nMapID)
    : ServerObject(uidf::buildMapUID(nMapID))
    , m_ID(nMapID)
    , m_mir2xMapData(*([nMapID]() -> Mir2xMapData *
      {
          // server is multi-thread
          // but creating server map is always in service core

          if(auto pMir2xMapData = g_mapBinDB->Retrieve(nMapID)){
              return pMir2xMapData;
          }

          // when constructing a servermap
          // servicecore should test if current nMapID valid
          throw fflerror("load map failed: ID = %d, Name = %s", nMapID, DBCOM_MAPRECORD(nMapID).Name);
      }()))
    , m_serviceCore(pServiceCore)
    , m_cellVec2D()
    , m_luaModule(nullptr)
{
    if(!m_mir2xMapData.Valid()){
        throw fflerror("load map failed: ID = %d, Name = %s", nMapID, DBCOM_MAPRECORD(nMapID).Name);
    }

    m_cellVec2D.resize(W());
    m_cellVec2D.shrink_to_fit();

    for(auto &rstStateLine: m_cellVec2D){
        rstStateLine.resize(H());
        rstStateLine.shrink_to_fit();
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
                g_monoServer->addLog(LOGTYPE_FATAL, "Unsupported message: %s", rstMPK.Name());
                break;
            }
    }
}

bool ServerMap::groundValid(int nX, int nY) const
{
    return true
        && m_mir2xMapData.Valid()
        && m_mir2xMapData.ValidC(nX, nY)
        && m_mir2xMapData.Cell(nX, nY).CanThrough();
}

bool ServerMap::canMove(bool bCheckCO, bool bCheckLock, int nX, int nY) const
{
    if(groundValid(nX, nY)){
        if(bCheckCO){
            for(auto nUID: GetUIDListRef(nX, nY)){
                if(auto nType = uidf::getUIDType(nUID); nType == UID_PLY || nType == UID_MON){
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
                throw fflerror("invalid CheckCO provided: %d, should be (0, 1, 2)", nCheckCO);
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
                throw fflerror("invalid CheckLock provided: %d, should be (0, 1, 2)", nCheckLock);
            }
    }

    int nMaxIndex = -1;
    switch(mathf::LDistance2(nX0, nY0, nX1, nY1)){
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

    const int nDX = (nX1 > nX0) - (nX1 < nX0);
    const int nDY = (nY1 > nY0) - (nY1 < nY0);

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
                    throw fflerror("invalid grid provided: %d at (%d, %d)", nGrid, nX0 + nDX * nIndex, nY0 + nDY * nIndex);
                }
        }
    }

    return 1.00 + nMaxIndex * 0.10 + fExtraPen;
}

std::tuple<bool, int, int> ServerMap::GetValidGrid(bool bCheckCO, bool bCheckLock, int nCheckCount) const
{
    for(int nIndex = 0; (nCheckCount <= 0) || (nIndex < nCheckCount); ++nIndex){
        int nX = std::rand() % W();
        int nY = std::rand() % H();

        if(In(ID(), nX, nY) && canMove(bCheckCO, bCheckLock, nX, nY)){
            return {true, nX, nY};
        }
    }
    return {false, -1, -1};
}

std::tuple<bool, int, int> ServerMap::GetValidGrid(bool bCheckCO, bool bCheckLock, int nCheckCount, int nX, int nY) const
{
    if(!In(ID(), nX, nY)){
        throw fflerror("invalid location: (%d, %d)", nX, nY);
    }

    RotateCoord stRC(nX, nY, 0, 0, W(), H());
    for(int nIndex = 0; (nCheckCount <= 0) || (nIndex < nCheckCount); ++nIndex){

        int nCurrX = stRC.X();
        int nCurrY = stRC.Y();

        if(In(ID(), nCurrX, nCurrY) && canMove(bCheckCO, bCheckLock, nCurrX, nCurrY)){
            return {true, nCurrX, nCurrY};
        }

        if(!stRC.forward()){
            return {false, -1, -1};
        }
    }
    return {false, -1, -1};
}

uint64_t ServerMap::Activate()
{
    if(auto nUID = ServerObject::Activate()){
        delete m_luaModule;
        m_luaModule = new ServerMap::ServerMapLuaModule();
        RegisterLuaExport(m_luaModule);
        return nUID;
    }
    return 0;
}

void ServerMap::AddGridUID(uint64_t nUID, int nX, int nY, bool bForce)
{
    if(!ValidC(nX, nY)){
        throw fflerror("invalid location: (%d, %d)", nX, nY);
    }

    if(bForce || groundValid(nX, nY)){
        if(auto &rstUIDList = GetUIDListRef(nX, nY); std::find(rstUIDList.begin(), rstUIDList.end(), nUID) == rstUIDList.end()){
            rstUIDList.push_back(nUID);
        }
    }
}

void ServerMap::RemoveGridUID(uint64_t nUID, int nX, int nY)
{
    if(!ValidC(nX, nY)){
        throw fflerror("invalid location: (%d, %d)", nX, nY);
    }

    auto &rstUIDList = GetUIDListRef(nX, nY);
    auto  pUIDRecord = std::find(rstUIDList.begin(), rstUIDList.end(), nUID);

    if(pUIDRecord != rstUIDList.end()){
        std::swap(rstUIDList.back(), *pUIDRecord);
        rstUIDList.pop_back();

        if(rstUIDList.size() * 2 < rstUIDList.capacity()){
            rstUIDList.shrink_to_fit();
        }
    }
}

bool ServerMap::doUIDList(int nX, int nY, const std::function<bool(uint64_t)> &fnOP)
{
    if(!ValidC(nX, nY)){
        return false;
    }

    if(!fnOP){
        return false;
    }

    for(auto nUID: GetUIDListRef(nX, nY)){
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

    if((nW > 0) && (nH > 0) && mathf::rectangleOverlapRegion(0, 0, W(), H(), &nX0, &nY0, &nW, &nH)){

        // get the clip region over the map
        // if no valid region we won't do the rest

        for(int nX = nX0; nX < nX0 + nW; ++nX){
            for(int nY = nY0; nY < nY0 + nH; ++nY){
                if(true || ValidC(nX, nY)){
                    if(mathf::LDistance2(nX, nY, nCX0, nCY0) <= (nCR - 1) * (nCR - 1)){
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
    if((nW > 0) && (nH > 0) && mathf::rectangleOverlapRegion(0, 0, W(), H(), &nX0, &nY0, &nW, &nH)){

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
            && mathf::rectangleOverlapRegion(0, 0, W(), H(), &nX0, &nY0, &nW, &nH)){

        // get the clip region over the map
        // if no valid region we won't do the rest

        RotateCoord stRC(nCX0, nCY0, nX0, nY0, nW, nH);
        do{
            const int nX = stRC.X();
            const int nY = stRC.Y();

            if(true || ValidC(nX, nY)){
                if(mathf::LDistance2(nX, nY, nCX0, nCY0) <= (nCR - 1) * (nCR - 1)){
                    if(!fnOP){
                        return false;
                    }

                    if(fnOP(nX, nY)){
                        return true;
                    }
                }
            }
        }while(stRC.forward());
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
            && mathf::rectangleOverlapRegion(0, 0, W(), H(), &nX0, &nY0, &nW, &nH)){

        // get the clip region over the map
        // if no valid region we won't do the rest

        RotateCoord stRC(nCX, nCY, nX0, nY0, nW, nH);
        do{
            const int nX = stRC.X();
            const int nY = stRC.Y();

            if(true || ValidC(nX, nY)){
                if(!fnOP){
                    return false;
                }

                if(fnOP(nX, nY)){
                    return true;
                }
            }
        }while(stRC.forward());
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
            && groundValid(nX, nY)){

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
                for(auto nUID: GetUIDListRef(nX, nY)){
                    if(uidf::getUIDType(nUID) == UID_PLY){
                        m_actorPod->forward(nUID, {MPK_SHOWDROPITEM, stAMSDI});
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
            for(auto nUID: GetUIDListRef(nX, nY)){
                if(uidf::getUIDType(nUID) == UID_MON){
                    if(nMonsterID){
                        nCount += ((uidf::getMonsterID(nUID) == nMonsterID) ? 1 : 0);
                    }else{
                        nCount++;
                    }
                }
            }
        }
    }
    return nCount;
}

void ServerMap::notifyNewCO(uint64_t nUID, int nX, int nY)
{
    AMNotifyNewCO stAMNNCO;
    std::memset(&stAMNNCO, 0, sizeof(stAMNNCO));

    stAMNNCO.UID = nUID;
    DoCircle(nX, nY, 20, [this, stAMNNCO](int nX, int nY) -> bool
    {
        if(true || ValidC(nX, nY)){
            doUIDList(nX, nY, [this, stAMNNCO](uint64_t nUID)
            {
                if(nUID != stAMNNCO.UID){
                    m_actorPod->forward(nUID, {MPK_NOTIFYNEWCO, stAMNNCO});
                }
                return false;
            });
        }
        return false;
    });
}

Monster *ServerMap::AddMonster(uint32_t nMonsterID, uint64_t nMasterUID, int nHintX, int nHintY, bool bStrictLoc)
{
    if(!ValidC(nHintX, nHintY)){
        if(bStrictLoc){
            return nullptr;
        }

        nHintX = std::rand() % W();
        nHintY = std::rand() % H();
    }

    if(auto [bDstOK, nDstX, nDstY] = GetValidGrid(false, false, (int)(bStrictLoc), nHintX, nHintY); bDstOK){
        auto pMonster = new Monster
        {
            nMonsterID,
            m_serviceCore,
            this,
            nDstX,
            nDstY,
            DIR_UP,
            nMasterUID,
        };

        pMonster->Activate();

        AddGridUID (pMonster->UID(), nDstX, nDstY, true);
        notifyNewCO(pMonster->UID(), nDstX, nDstY);

        return pMonster;
    }
    return nullptr;
}

NPChar *ServerMap::addNPChar(uint16_t npcID, int hintX, int hintY, int direction, bool strictLoc)
{
    if(!ValidC(hintX, hintY)){
        if(strictLoc){
            return nullptr;
        }

        hintX = std::rand() % W();
        hintY = std::rand() % H();
    }

    if(const auto [dstOK, dstX, dstY] = GetValidGrid(false, false, (int)(strictLoc), hintX, hintY); dstOK){
        auto pNPC = new NPChar
        {
            npcID,
            m_serviceCore,
            this,
            dstX,
            dstY,
            direction,
        };

        pNPC->Activate();

        AddGridUID (pNPC->UID(), dstX, dstY, true);
        notifyNewCO(pNPC->UID(), dstX, dstY);

        return pNPC;
    }
    return nullptr;
}

Player *ServerMap::AddPlayer(uint32_t nDBID, int nHintX, int nHintY, int nDirection, bool bStrictLoc)
{
    if(!ValidC(nHintX, nHintY)){
        if(bStrictLoc){
            return nullptr;
        }

        nHintX = std::rand() % W();
        nHintY = std::rand() % H();
    }

    if(auto [bDstOK, nDstX, nDstY] = GetValidGrid(false, false, (int)(bStrictLoc), nHintX, nHintY); bDstOK){
        auto pPlayer = new Player
        {
            nDBID,
            m_serviceCore,
            this,
            nDstX,
            nDstY,
            nDirection,
        };

        pPlayer->Activate();

        AddGridUID (pPlayer->UID(), nDstX, nDstY, true);
        notifyNewCO(pPlayer->UID(), nDstX, nDstY);

        return pPlayer;
    }
    return nullptr;
}

bool ServerMap::RegisterLuaExport(ServerMap::ServerMapLuaModule *pModule)
{
    if(!pModule){
        throw fflerror("invalid module: %p", pModule);
    }

    // load lua script to the module
    {
        auto szScriptPath = g_serverConfigureWindow->GetScriptPath();
        if(szScriptPath.empty()){
            szScriptPath = "script/map";
        }

        std::string szCommandFile = ((szScriptPath + "/") + DBCOM_MAPRECORD(ID()).Name) + ".lua";

        std::stringstream stCommand;
        std::ifstream stCommandFile(szCommandFile.c_str());

        stCommand << stCommandFile.rdbuf();
        pModule->LoadBatch(stCommand.str().c_str());
    }

    // register lua functions/variables related *this* map

    pModule->getLuaState().set_function("getMapID", [this]() -> int
    {
        return (int)(ID());
    });

    pModule->getLuaState().set_function("getMapName", [this]() -> std::string
    {
        return std::string(DBCOM_MAPRECORD(ID()).Name);
    });

    pModule->getLuaState().set_function("getRandLoc", [this]() /* -> ? */
    {
        std::array<int, 2> loc;
        while(true){
            const int x = std::rand() % W();
            const int y = std::rand() % H();

            if(groundValid(x, y)){
                loc[0] = x;
                loc[1] = y;
                break;
            }
        }
        return sol::as_returns(loc);
    });

    pModule->getLuaState().set_function("getMonsterCount", [this](sol::variadic_args stVariadicArgs) -> int
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

    pModule->getLuaState().set_function("addMonster", [this](sol::object stMonsterID, sol::variadic_args stVariadicArgs) -> bool
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
                    return AddMonster(nMonsterID, 0, -1, -1, false);
                }
            case 2:
                {
                    if(true
                            && stArgList[0].is<int>()
                            && stArgList[1].is<int>()){

                        auto nX = stArgList[0].as<int>();
                        auto nY = stArgList[1].as<int>();

                        return AddMonster(nMonsterID, 0, nX, nY, false);
                    }
                    break;
                }
            case 3:
                {
                    if(true
                            && stArgList[0].is<int >()
                            && stArgList[1].is<int >()
                            && stArgList[2].is<bool>()){

                        const auto nX = stArgList[0].as<int >();
                        const auto nY = stArgList[1].as<int >();
                        const auto bStrictLoc = stArgList[2].as<bool>();

                        return AddMonster(nMonsterID, 0, nX, nY, bStrictLoc);
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

    pModule->getLuaState().set_function("addNPC", [this](int npcID, sol::variadic_args args) -> bool
    {
        const auto fnUsage = [this]()
        {
            // addCWLog(nCWID, 2, ">>> ", "addNPC(NPCID: int, MapID: int)");
            // addCWLog(nCWID, 2, ">>> ", "addNPC(NPCID: int, MapID: int, X: int, Y: int)");
            // addCWLog(nCWID, 2, ">>> ", "addNPC(NPCID: int, MapID: int, X: int, Y: int, Random: bool)");
        };

        const std::vector<sol::object> argList(args.begin(), args.end());
        switch(argList.size()){
            case 0:
                {
                    return addNPChar((uint16_t)(npcID), -1, -1, 0, false);
                }
            case 2:
                {
                    if(argList[0].is<int>() && argList[1].is<int>()){
                        return addNPChar((uint32_t)(npcID), argList[0].as<int>(), argList[1].as<int>(), 0, false);
                    }
                    break;
                }
            case 3:
                {
                    if(argList[0].is<int>() && argList[1].is<int>() && argList[2].is<bool>()){
                        return addNPChar((uint32_t)(npcID), argList[0].as<int>(), argList[1].as<int>(), 0, argList[2].as<bool>());
                    }
                    break;
                }
            case 4:
                {
                    if(argList[0].is<int>() && argList[1].is<int>() && argList[2].is<int>() && argList[3].is<bool>()){
                        return addNPChar((uint32_t)(npcID), argList[0].as<int>(), argList[1].as<int>(), argList[2].is<int>(), argList[3].as<bool>());
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }

        fnUsage();
        return false;
    });

    return true;
}

int ServerMap::CheckPathGrid(int nX, int nY) const
{
    if(!m_mir2xMapData.ValidC(nX, nY)){
        return PathFind::INVALID;
    }

    if(!m_mir2xMapData.Cell(nX, nY).CanThrough()){
        return PathFind::OBSTACLE;
    }

    // for(auto nUID: GetUIDListRef(nX, nY)){
    //     if(auto nType = uidf::getUIDType(nUID); nType == UID_PLY || nType == UID_MON){
    //         return PatFind::OCCUPIED;
    //     }
    // }

    if(!GetUIDListRef(nX, nY).empty()){
        return PathFind::OCCUPIED;
    }

    if(GetCell(nX, nY).Locked){
        return PathFind::LOCKED;
    }

    return PathFind::FREE;
}
