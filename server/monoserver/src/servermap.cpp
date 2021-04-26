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
#include "totype.hpp"
#include "uidf.hpp"
#include "npchar.hpp"
#include "player.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "actorpod.hpp"
#include "taodog.hpp"
#include "guard.hpp"
#include "filesys.hpp"
#include "taoskeleton.hpp"
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
#include "serverargparser.hpp"
#include "serverconfigurewindow.hpp"

extern MapBinDB *g_mapBinDB;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;
extern ServerConfigureWindow *g_serverConfigureWindow;

ServerMap::ServerMapLuaModule::ServerMapLuaModule(ServerMap *mapPtr)
{
    if(!mapPtr){
        throw fflerror("ServerMapLuaModule binds to empty ServerMap");
    }

    getLuaState().set_function("getMapID", [mapPtr]() -> int
    {
        return to_d(mapPtr->ID());
    });

    getLuaState().set_function("getMapName", [mapPtr]() -> std::string
    {
        return std::string(to_cstr(DBCOM_MAPRECORD(mapPtr->ID()).name));
    });

    getLuaState().set_function("getMapSize", [mapPtr]()
    {
        return sol::as_returns(std::vector<int>{mapPtr->W(), mapPtr->H()});
    });

    getLuaState().set_function("getCanThroughGridCount", [mapPtr, gridCount = to_d(-1)]() mutable -> int
    {
        if(gridCount >= 0){
            return gridCount;
        }

        gridCount = 0;
        for(int x = 0; x < mapPtr->W(); ++x){
            for(int y = 0; y < mapPtr->H(); ++y){
                if(mapPtr->groundValid(x, y)){
                    gridCount++;
                }
            }
        }
        return gridCount;
    });

    getLuaState().set_function("getRandLoc", [mapPtr]() /* -> ? */
    {
        std::array<int, 2> loc;
        while(true){
            const int x = std::rand() % mapPtr->W();
            const int y = std::rand() % mapPtr->H();

            if(mapPtr->groundValid(x, y)){
                loc[0] = x;
                loc[1] = y;
                break;
            }
        }
        return sol::as_returns(loc);
    });

    getLuaState().set_function("getMonsterCount", [mapPtr](sol::variadic_args args) -> int
    {
        const std::vector<sol::object> argList(args.begin(), args.end());
        switch(argList.size()){
            case 0:
                {
                    return mapPtr->getMonsterCount(0);
                }
            case 1:
                {
                    if(argList[0].is<int>()){
                        if(const int monID = argList[0].as<int>(); monID >= 0){
                            return mapPtr->getMonsterCount(monID);
                        }
                    }

                    else if(argList[0].is<std::string>()){
                        if(const int monID = DBCOM_MONSTERID(to_u8cstr(argList[0].as<std::string>().c_str())); monID >= 0){
                            return mapPtr->getMonsterCount(monID);
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

    getLuaState().set_function("addMonster", [mapPtr](sol::object monInfo, sol::variadic_args args) -> bool
    {
        const uint32_t monID = [&monInfo]() -> uint32_t
        {
            if(monInfo.is<int>()){
                return monInfo.as<int>();
            }

            if(monInfo.is<std::string>()){
                return DBCOM_MONSTERID(to_u8cstr(monInfo.as<std::string>().c_str()));
            }

            return 0;
        }();

        if(!monID){
            return false;
        }

        const std::vector<sol::object> argList(args.begin(), args.end());
        switch(argList.size()){
            case 0:
                {
                    return mapPtr->addMonster(monID, 0, -1, -1, false);
                }
            case 2:
                {
                    if(true
                            && argList[0].is<int>()
                            && argList[1].is<int>()){

                        auto nX = argList[0].as<int>();
                        auto nY = argList[1].as<int>();

                        return mapPtr->addMonster(monID, 0, nX, nY, false);
                    }
                    break;
                }
            case 3:
                {
                    if(true
                            && argList[0].is<int >()
                            && argList[1].is<int >()
                            && argList[2].is<bool>()){

                        const auto nX = argList[0].as<int >();
                        const auto nY = argList[1].as<int >();
                        const auto bStrictLoc = argList[2].as<bool>();

                        return mapPtr->addMonster(monID, 0, nX, nY, bStrictLoc);
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

    getLuaState().set_function("addGuard", [mapPtr](std::string type, int x, int y, int direction) -> bool
    {
        const uint32_t monID = DBCOM_MONSTERID(to_u8cstr(type));
        if(!monID){
            return false;
        }
        return mapPtr->addGuard(monID, x, y, direction);
    });

    getLuaState().script_file([mapPtr]() -> std::string
    {
        const auto configScriptPath = g_serverConfigureWindow->getScriptPath();
        const auto scriptPath = configScriptPath.empty() ? std::string("script/map") : configScriptPath;

        const auto scriptName = str_printf("%s/%s.lua", scriptPath.c_str(), to_cstr(DBCOM_MAPRECORD(mapPtr->ID()).name));
        if(filesys::hasFile(scriptName.c_str())){
            return scriptName;
        }

        const auto defaultScriptName = scriptPath + "/default.lua";
        if(filesys::hasFile(defaultScriptName.c_str())){
            return defaultScriptName;
        }
        throw fflerror("can't load proper script for map %s", to_cstr(DBCOM_MAPRECORD(mapPtr->ID()).name));
    }());

    m_coHandler = getLuaState()["main"];
    if(!m_coHandler){
        throw fflerror("can't load lua entry function: main()");
    }

    // checkResult(m_coHandler());
}

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
          throw fflerror("load map failed: ID = %d, Name = %s", nMapID, to_cstr(DBCOM_MAPRECORD(nMapID).name));
      }()))
    , m_serviceCore(pServiceCore)
{
    if(!m_mir2xMapData.Valid()){
        throw fflerror("load map failed: ID = %d, Name = %s", nMapID, to_cstr(DBCOM_MAPRECORD(nMapID).name));
    }

    m_gridList.resize(W());
    m_gridList.shrink_to_fit();

    for(auto &gridLine: m_gridList){
        gridLine.resize(H());
        gridLine.shrink_to_fit();
    }

    for(const auto &entry: DBCOM_MAPRECORD(nMapID).linkArray){
        if(true
                && entry.w > 0
                && entry.h > 0
                && validC(entry.x, entry.y)){

            for(int nW = 0; nW < entry.w; ++nW){
                for(int nH = 0; nH < entry.h; ++nH){
                    getGrid(entry.x + nW, entry.y + nH).mapID   = DBCOM_MAPID(entry.endName);
                    getGrid(entry.x + nW, entry.y + nH).switchX = entry.endX;
                    getGrid(entry.x + nW, entry.y + nH).switchY = entry.endY;
                }
            }
        }else{
            break;
        }
    }
}

void ServerMap::operateAM(const ActorMsgPack &rstMPK)
{
    switch(rstMPK.type()){
        case AM_PICKUP:
            {
                on_AM_PICKUP(rstMPK);
                break;
            }
        case AM_NEWDROPITEM:
            {
                on_AM_NEWDROPITEM(rstMPK);
                break;
            }
        case AM_TRYLEAVE:
            {
                on_AM_TRYLEAVE(rstMPK);
                break;
            }
        case AM_UPDATEHP:
            {
                on_AM_UPDATEHP(rstMPK);
                break;
            }
        case AM_DEADFADEOUT:
            {
                on_AM_DEADFADEOUT(rstMPK);
                break;
            }
        case AM_ACTION:
            {
                on_AM_ACTION(rstMPK);
                break;
            }
        case AM_BADACTORPOD:
            {
                on_AM_BADACTORPOD(rstMPK);
                break;
            }
        case AM_TRYMOVE:
            {
                on_AM_TRYMOVE(rstMPK);
                break;
            }
        case AM_PATHFIND:
            {
                on_AM_PATHFIND(rstMPK);
                break;
            }
        case AM_TRYMAPSWITCH:
            {
                on_AM_TRYMAPSWITCH(rstMPK);
                break;
            }
        case AM_METRONOME:
            {
                on_AM_METRONOME(rstMPK);
                break;
            }
        case AM_TRYSPACEMOVE:
            {
                on_AM_TRYSPACEMOVE(rstMPK);
                break;
            }
        case AM_ADDCHAROBJECT:
            {
                on_AM_ADDCHAROBJECT(rstMPK);
                break;
            }
        case AM_PULLCOINFO:
            {
                on_AM_PULLCOINFO(rstMPK);
                break;
            }
        case AM_QUERYCOCOUNT:
            {
                on_AM_QUERYCOCOUNT(rstMPK);
                break;
            }
        case AM_OFFLINE:
            {
                on_AM_OFFLINE(rstMPK);
                break;
            }
        default:
            {
                g_monoServer->addLog(LOGTYPE_FATAL, "Unsupported message: %s", mpkName(rstMPK.type()));
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
            for(auto nUID: getUIDList(nX, nY)){
                if(auto nType = uidf::getUIDType(nUID); nType == UID_PLY || nType == UID_MON){
                    return false;
                }
            }
        }

        if(bCheckLock){
            if(getGrid(nX, nY).locked){
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

void ServerMap::addGridUID(uint64_t uid, int nX, int nY, bool bForce)
{
    if(!validC(nX, nY)){
        throw fflerror("invalid location: (%d, %d)", nX, nY);
    }

    if(bForce || groundValid(nX, nY)){
        if(!hasGridUID(uid, nX, nY)){
            getUIDList(nX, nY).push_back(uid);
        }
    }
}

bool ServerMap::hasGridUID(uint64_t uid, int nX, int nY) const
{
    if(!validC(nX, nY)){
        throw fflerror("invalid location: (%d, %d)", nX, nY);
    }

    const auto &uidList = getUIDList(nX, nY);
    return std::find(uidList.begin(), uidList.end(), uid) != uidList.end();
}

void ServerMap::removeGridUID(uint64_t uid, int nX, int nY)
{
    if(!validC(nX, nY)){
        throw fflerror("invalid location: (%d, %d)", nX, nY);
    }

    auto &uidList = getUIDList(nX, nY);
    auto p = std::find(uidList.begin(), uidList.end(), uid);

    if(p == uidList.end()){
        return;
    }

    std::swap(uidList.back(), *p);
    uidList.pop_back();

    if(uidList.size() * 2 < uidList.capacity()){
        uidList.shrink_to_fit();
    }
}

bool ServerMap::DoCenterCircle(int nCX0, int nCY0, int nCR, bool bPriority, const std::function<bool(int, int)> &fnOP)
{
    if(!bPriority){
        return doCircle(nCX0, nCY0, nCR, fnOP);
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

            if(validC(nX, nY)){
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
        return doSquare(nCX - nW / 2, nCY - nH / 2, nW, nH, fnOP);
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

            if(validC(nX, nY)){
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

bool ServerMap::hasGridItemID(uint32_t itemID, int x, int y) const
{
    return getGridItemIDCount(itemID, x, y) > 0;
}

size_t ServerMap::getGridItemIDCount(uint32_t itemID, int x, int y) const
{
    if(!validC(x, y)){
        return 0;
    }

    size_t count = 0;
    for(const auto id: getGridItemIDList(x, y)){
        if(id == itemID){
            count++;
        }
    }
    return count;
}

void ServerMap::addGridItemID(uint32_t itemID, int x, int y, bool post)
{
    if(!(DBCOM_ITEMRECORD(itemID) && groundValid(x, y))){
        throw fflerror("invalid arguments: itemID = %llu, x = %d, y = %d", to_llu(itemID), x, y);
    }

    getGridItemIDList(x, y).push_back(itemID);
    if(post){
        postGridItemIDList(x, y);
    }
}

void ServerMap::removeGridItemID(uint32_t itemID, int x, int y, bool post)
{
    if(!hasGridItemID(itemID, x, y)){
        return;
    }

    auto &itemIDList = getGridItemIDList(x, y);
    for(int i = to_d(itemIDList.size()) - 1; i >= 0; --i){
        if(itemIDList[i] == itemID){
            itemIDList.erase(itemIDList.begin() + i);
            if(post){
                postGridItemIDList(x, y);
            }
            return;
        }
    }
}

SDGroundItemIDList ServerMap::getGroundItemIDList(int x, int y, size_t r) const
{
    if(r <= 0){
        throw fflerror("invalid cover radus: 0");
    }

    // center (x, y) can be invalid
    // an invalid center can cover valid grids

    SDGroundItemIDList groundItemIDList;
    groundItemIDList.mapID = ID();

    doCircle(x, y, r, [&groundItemIDList, this](int x, int y) -> bool
    {
        if(groundValid(x, y)){
            groundItemIDList.gridItemIDList.push_back(SDGroundItemIDList::GridItemIDList
            {
                .x = x,
                .y = y,
                .itemIDList = getGridItemIDList(x, y),
            });
        }
        return false;
    });
    return groundItemIDList;
}

void ServerMap::postGridItemIDList(int x, int y)
{
    if(!groundValid(x, y)){
        throw fflerror("invalid arguments: x = %d, y = %d", x, y);
    }

    const auto sdBuf = cerealf::serialize(getGroundItemIDList(x, y, 1), true);
    doCircle(x, y, 20, [&sdBuf, this](int x, int y) -> bool
    {
        if(groundValid(x, y)){
            doUIDList(x, y, [&sdBuf, this](uint64_t uid) -> bool
            {
                if(uidf::getUIDType(uid) == UID_PLY){
                    sendNetPackage(uid, SM_GROUNDITEMIDLIST, sdBuf);
                }
                return false;
            });
        }
        return false;
    });
}

void ServerMap::postGroundItemIDList(uint64_t uid, int x, int y)
{
    if(!groundValid(x, y)){
        throw fflerror("invalid arguments: x = %d, y = %d", x, y);
    }

    if(uidf::getUIDType(uid) != UID_PLY){
        throw fflerror("post ground item list to non-player: %s", uidf::getUIDTypeCStr(uid));
    }
    sendNetPackage(uid, SM_GROUNDITEMIDLIST, cerealf::serialize(getGroundItemIDList(x, y, 20)));
}

int ServerMap::getMonsterCount(uint32_t monsterID)
{
    int result = 0;
    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){
            for(auto nUID: getUIDList(nX, nY)){
                if(uidf::getUIDType(nUID) == UID_MON){
                    if(monsterID){
                        result += ((uidf::getMonsterID(nUID) == monsterID) ? 1 : 0);
                    }
                    else{
                        result++;
                    }
                }
            }
        }
    }
    return result;
}

void ServerMap::notifyNewCO(uint64_t nUID, int nX, int nY)
{
    AMNotifyNewCO amNNCO;
    std::memset(&amNNCO, 0, sizeof(amNNCO));

    amNNCO.UID = nUID;
    doCircle(nX, nY, 20, [this, amNNCO](int nX, int nY) -> bool
    {
        if(true || validC(nX, nY)){
            doUIDList(nX, nY, [this, amNNCO](uint64_t nUID)
            {
                if(nUID != amNNCO.UID){
                    m_actorPod->forward(nUID, {AM_NOTIFYNEWCO, amNNCO});
                }
                return false;
            });
        }
        return false;
    });
}

Monster *ServerMap::addMonster(uint32_t nMonsterID, uint64_t nMasterUID, int nHintX, int nHintY, bool bStrictLoc)
{
    if(uidf::getUIDType(nMasterUID) == UID_PLY){
        if(g_serverArgParser->disablePetSpawn){
            return nullptr;
        }
    }
    else{
        if(g_serverArgParser->disableMonsterSpawn){
            return nullptr;
        }
    }

    if(!validC(nHintX, nHintY)){
        if(bStrictLoc){
            return nullptr;
        }

        nHintX = std::rand() % W();
        nHintY = std::rand() % H();
    }

    if(auto [bDstOK, nDstX, nDstY] = GetValidGrid(false, false, to_d(bStrictLoc), nHintX, nHintY); bDstOK){
        Monster *monsterPtr = nullptr;
        switch(nMonsterID){
            case DBCOM_MONSTERID(u8"变异骷髅"):
                {
                    monsterPtr = new TaoSkeleton
                    {
                        m_serviceCore,
                        this,
                        nDstX,
                        nDstY,
                        nMasterUID,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"神兽"):
                {
                    monsterPtr = new TaoDog
                    {
                        m_serviceCore,
                        this,
                        nDstX,
                        nDstY,
                        DIR_UP, // TODO face its master
                        nMasterUID,
                    };
                    break;
                }
            default:
                {
                    monsterPtr = new Monster
                    {
                        nMonsterID,
                        m_serviceCore,
                        this,
                        nDstX,
                        nDstY,
                        DIR_UP,
                        nMasterUID,
                    };
                    break;
                }
        }

        monsterPtr->activate();
        return monsterPtr;
    }
    return nullptr;
}

Guard *ServerMap::addGuard(uint32_t monID, int x, int y, int direction)
{
    fflassert(monID);
    fflassert(validC(x, y));

    auto guardPtr = new Guard
    {
        monID,
        m_serviceCore,
        this,
        x,
        y,
        direction,
    };

    guardPtr->activate();
    return guardPtr;
}

NPChar *ServerMap::addNPChar(const SDInitNPChar &initParam)
{
    try{
        auto npcPtr = new NPChar(m_serviceCore, this, std::make_unique<NPChar::LuaNPCModule>(initParam));
        npcPtr->activate();
        return npcPtr;
    }
    catch(const std::exception &e){
        g_monoServer->addLog(LOGTYPE_WARNING, "failed to ServerMap::addNPChar: %s", e.what());
        return nullptr;
    }
    catch(...){
        g_monoServer->addLog(LOGTYPE_WARNING, "failed to ServerMap::addNPChar");
        return nullptr;
    }
}

Player *ServerMap::addPlayer(const SDInitPlayer &initPlayer)
{
    int nHintX = initPlayer.x;
    int nHintY = initPlayer.y;
    bool bStrictLoc = false;

    if(!validC(nHintX, nHintY)){
        if(bStrictLoc){
            return nullptr;
        }

        nHintX = std::rand() % W();
        nHintY = std::rand() % H();
    }

    if(auto [bDstOK, nDstX, nDstY] = GetValidGrid(false, false, to_d(bStrictLoc), nHintX, nHintY); bDstOK){
        auto playerPtr = new Player
        {
            initPlayer,
            m_serviceCore,
            this,
        };

        playerPtr->activate();
        return playerPtr;
    }
    return nullptr;
}

int ServerMap::CheckPathGrid(int nX, int nY) const
{
    if(!m_mir2xMapData.ValidC(nX, nY)){
        return PathFind::INVALID;
    }

    if(!m_mir2xMapData.Cell(nX, nY).CanThrough()){
        return PathFind::OBSTACLE;
    }

    // for(auto nUID: getUIDList(nX, nY)){
    //     if(auto nType = uidf::getUIDType(nUID); nType == UID_PLY || nType == UID_MON){
    //         return PatFind::OCCUPIED;
    //     }
    // }

    if(!getUIDList(nX, nY).empty()){
        return PathFind::OCCUPIED;
    }

    if(getGrid(nX, nY).locked){
        return PathFind::LOCKED;
    }

    return PathFind::FREE;
}

void ServerMap::loadNPChar()
{
    const auto cfgScriptPath = g_serverConfigureWindow->getScriptPath();
    const auto scriptPath = cfgScriptPath.empty() ? std::string("script/npc") : cfgScriptPath;

    const auto reg = str_printf("%s\\..*\\.lua", to_cstr(DBCOM_MAPRECORD(ID()).name));
    for(const auto &fileName: filesys::getFileList(scriptPath.c_str(), reg.c_str())){
        // file as: "道馆.铁匠.lua"
        // parse the string to get "铁匠" as npc name
        const auto p1 = fileName.find('.');
        fflassert(p1 != std::string::npos);

        const auto p2 = fileName.find('.', p1 + 1);
        fflassert(p2 != std::string::npos);

        addNPChar(SDInitNPChar
        {
            .filePath = scriptPath,
            .mapID    = ID(),
            .npcName  = fileName.substr(p1 + 1, p2 - p1 - 1),
        });
    }
}
