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
#include "cannibalplant.hpp"
#include "bugbatmaggot.hpp"
#include "monstertree.hpp"
#include "raiitimer.hpp"
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

    getLuaState().set_function("getRandLoc", [mapPtr]()
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

    getLuaState().set_function("countGLoc", [fullCount = -1, mapPtr](sol::variadic_args args) mutable
    {
        const std::vector<sol::object> argList(args.begin(), args.end());
        const auto [useFullMap, regionX, regionY, regionW, regionH] = [&argList, mapPtr]() -> std::tuple<bool, int, int, int, int>
        {
            switch(argList.size()){
                case 0:
                    {
                        return
                        {
                            true,
                            0,
                            0,
                            mapPtr->W(),
                            mapPtr->H(),
                        };
                    }
                case 4:
                    {
                        return
                        {
                            false,
                            argList[0].as<int>(),
                            argList[1].as<int>(),
                            argList[2].as<int>(),
                            argList[3].as<int>(),
                        };
                    }
                default:
                    {
                        throw fflerror("invalid argument count: %zu", argList.size());
                    }
            }
        }();

        fflassert(regionW > 0);
        fflassert(regionH > 0);

        int roiX = regionX;
        int roiY = regionY;
        int roiW = regionW;
        int roiH = regionH;

        if(!mathf::rectangleOverlapRegion<int>(0, 0, mapPtr->W(), mapPtr->H(), roiX, roiY, roiW, roiH)){
            throw fflerror("invalid region: map = %s, x = %d, y = %d, w = %d, h = %d", to_cstr(DBCOM_MAPRECORD(mapPtr->ID()).name), regionX, regionY, regionW, regionH);
        }

        int count = 0;
        for(int yi = roiY; yi < roiY + roiH; ++yi){
            for(int xi = roiX; xi < roiX + roiW; ++xi){
                if(mapPtr->groundValid(xi, yi)){
                    count++;
                }
            }
        }

        if(useFullMap){
            fullCount = count; // cache
        }
        return count;
    });

    getLuaState().set_function("randGLoc", [mapPtr](sol::variadic_args args)
    {
        const std::vector<sol::object> argList(args.begin(), args.end());
        const auto [useFullMap, regionX, regionY, regionW, regionH] = [&argList, mapPtr]() -> std::tuple<bool, int, int, int, int>
        {
            switch(argList.size()){
                case 0:
                    {
                        return
                        {
                            true,
                            0,
                            0,
                            mapPtr->W(),
                            mapPtr->H(),
                        };
                    }
                case 4:
                    {
                        return
                        {
                            false,
                            argList[0].as<int>(),
                            argList[1].as<int>(),
                            argList[2].as<int>(),
                            argList[3].as<int>(),
                        };
                    }
                default:
                    {
                        throw fflerror("invalid argument count: %zu", argList.size());
                    }
            }
        }();

        if(const auto locopt = mapPtr->GetValidGrid(false, false, -1, regionX, regionY, regionW, regionH); locopt.has_value()){
            return sol::as_returns(std::array<int, 2>
            {
                std::get<0>(locopt.value()),
                std::get<1>(locopt.value()),
            });
        }

        // give detailed failure message
        // need it to validate map monster gen coroutine

        if(useFullMap){
            throw fflerror("no valid grid on map: map = %s", to_cstr(DBCOM_MAPRECORD(mapPtr->ID()).name));
        }
        else{
            throw fflerror("no valid grid in region: map = %s, x = %d, y = %d, w = %d, h = %d", to_cstr(DBCOM_MAPRECORD(mapPtr->ID()).name), regionX, regionY, regionW, regionH);
        }
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

    getLuaState().set_function("addMonster", [mapPtr](sol::object monInfo, sol::variadic_args args, sol::this_state s) -> sol::object
    {
        sol::state_view sv(s);
        const auto fnGetLuaUIDString = [&sv](const Monster *monPtr) -> sol::object
        {
            if(monPtr){
                return sol::object(sv, sol::in_place_type<std::string>, std::to_string(monPtr->UID()));
            }
            return sol::make_object(sv, sol::nil);
        };

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
            return fnGetLuaUIDString(nullptr);
        }

        const std::vector<sol::object> argList(args.begin(), args.end());
        switch(argList.size()){
            case 0:
                {
                    return fnGetLuaUIDString(mapPtr->addMonster(monID, 0, -1, -1, false));
                }
            case 2:
                {
                    if(true
                            && argList[0].is<int>()
                            && argList[1].is<int>()){

                        const auto nX = argList[0].as<int>();
                        const auto nY = argList[1].as<int>();
                        return fnGetLuaUIDString(mapPtr->addMonster(monID, 0, nX, nY, false));
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
                        return fnGetLuaUIDString(mapPtr->addMonster(monID, 0, nX, nY, bStrictLoc));
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }
        return fnGetLuaUIDString(nullptr);
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
        const auto configScriptPath = g_serverConfigureWindow->getConfig().scriptPath;
        const auto scriptPath = configScriptPath.empty() ? std::string("script/map") : (configScriptPath + "/map");

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

ServerMap::ServerMap(uint32_t mapID)
    : ServerObject(uidf::getMapUID(mapID))
    , m_ID(mapID)
    , m_mir2xMapData([mapID]()
      {
          Mir2xMapData data;
          if(auto p = g_mapBinDB->retrieve(mapID)){
              data = *p;
          }

          if(!data.valid()){
              throw fflerror("load map failed: ID = %d, Name = %s", to_d(mapID), to_cstr(DBCOM_MAPRECORD(mapID).name));
          }
          return data;
      }())
{
    m_gridList.resize(W() * H());
    for(const auto &entry: DBCOM_MAPRECORD(mapID).mapSwitchList){
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
        case AM_DROPITEM:
            {
                on_AM_DROPITEM(rstMPK);
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
        case AM_TRYJUMP:
            {
                on_AM_TRYJUMP(rstMPK);
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
        case AM_CASTFIREWALL:
            {
                on_AM_CASTFIREWALL(rstMPK);
                break;
            }
        case AM_ADDCO:
            {
                on_AM_ADDCO(rstMPK);
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
        case AM_STRIKEFIXEDLOCDAMAGE:
            {
                on_AM_STRIKEFIXEDLOCDAMAGE(rstMPK);
                break;
            }
        default:
            {
                throw fflerror("unsupported message: %s", mpkName(rstMPK.type()));
            }
    }
}

bool ServerMap::groundValid(int nX, int nY) const
{
    return m_mir2xMapData.validC(nX, nY) && m_mir2xMapData.cell(nX, nY).land.canThrough();
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

std::optional<std::tuple<int, int>> ServerMap::getRCGLoc(bool checkCO, bool checkLock, int checkCount, int centerX, int centerY, int regionX, int regionY, int regionW, int regionH) const
{
    fflassert(regionW > 0);
    fflassert(regionH > 0);

    int roiX = regionX;
    int roiY = regionY;
    int roiW = regionW;
    int roiH = regionH;

    if(!mathf::rectangleOverlapRegion<int>(0, 0, W(), H(), roiX, roiY, roiW, roiH)){
        throw fflerror("invalid region: map = %s, x = %d, y = %d, w = %d, h = %d", to_cstr(DBCOM_MAPRECORD(ID()).name), regionX, regionY, regionW, regionH);
    }

    RotateCoord rc
    {
        centerX,
        centerY,
        roiX,
        roiY,
        roiW,
        roiH,
    };

    for(int i = 0; (checkCount <= 0) || (i < checkCount); ++i){
        const int currX = rc.x();
        const int currY = rc.y();

        if(In(ID(), currX, currY) && canMove(checkCO, checkLock, currX, currY)){
            return std::make_tuple(currX, currY);
        }

        if(!rc.forward()){
            break;
        }
    }
    return {};
}

std::optional<std::tuple<int, int>> ServerMap::GetValidGrid(bool checkCO, bool checkLock, int checkCount) const
{
    return GetValidGrid(checkCO, checkLock, checkCount, 0, 0, W(), H());
}

std::optional<std::tuple<int, int>> ServerMap::GetValidGrid(bool checkCO, bool checkLock, int checkCount, int startX, int startY) const
{
    return getRCGLoc(checkCO, checkLock, checkCount, startX, startY, 0, 0, W(), H());
}

std::optional<std::tuple<int, int>> ServerMap::GetValidGrid(bool checkCO, bool checkLock, int checkCount, int regionX, int regionY, int regionW, int regionH) const
{
    fflassert(regionW > 0);
    fflassert(regionH > 0);
    return getRCGLoc(checkCO, checkLock, checkCount, regionX + std::rand() % regionW, regionY + std::rand() % regionH, regionX, regionY, regionW, regionH);
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

bool ServerMap::removeGridUID(uint64_t uid, int nX, int nY)
{
    if(!validC(nX, nY)){
        throw fflerror("invalid location: (%d, %d)", nX, nY);
    }

    auto &uidList = getUIDList(nX, nY);
    auto p = std::find(uidList.begin(), uidList.end(), uid);

    if(p == uidList.end()){
        return false;
    }

    std::swap(uidList.back(), *p);
    uidList.pop_back();

    if(uidList.size() * 2 < uidList.capacity()){
        uidList.shrink_to_fit();
    }
    return true;
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
            && mathf::rectangleOverlapRegion(0, 0, W(), H(), nX0, nY0, nW, nH)){

        // get the clip region over the map
        // if no valid region we won't do the rest

        RotateCoord rc(nCX0, nCY0, nX0, nY0, nW, nH);
        do{
            const int nX = rc.x();
            const int nY = rc.y();

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
        }while(rc.forward());
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
            && mathf::rectangleOverlapRegion(0, 0, W(), H(), nX0, nY0, nW, nH)){

        // get the clip region over the map
        // if no valid region we won't do the rest

        RotateCoord rc(nCX, nCY, nX0, nY0, nW, nH);
        do{
            const int nX = rc.x();
            const int nY = rc.y();

            if(validC(nX, nY)){
                if(!fnOP){
                    return false;
                }

                if(fnOP(nX, nY)){
                    return true;
                }
            }
        }while(rc.forward());
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

void ServerMap::addGridItem(SDItem item, int x, int y, bool post)
{
    fflassert(item);
    fflassert(groundValid(x, y));

    getGridItemList(x, y).push_back(DroppedItemNode
    {
        .item = std::move(item),
        .dropTime = hres_tstamp().to_msec(),
    });

    if(post){
        postGridItemIDList(x, y);
    }
}

void ServerMap::addGridItemID(uint32_t itemID, int x, int y, bool post)
{
    fflassert(DBCOM_ITEMRECORD(itemID));
    fflassert(groundValid(x, y));

    getGridItemList(x, y).push_back(DroppedItemNode
    {
        .item
        {
            .itemID = itemID,
            .seqID  = 1,
        },
        .dropTime = hres_tstamp().to_msec(),
    });

    if(post){
        postGridItemIDList(x, y);
    }
}

void ServerMap::removeGridItemID(uint32_t itemID, int x, int y, bool post)
{
    if(!hasGridItemID(itemID, x, y)){
        return;
    }

    auto &itemList = getGridItemList(x, y);
    for(auto p = itemList.rbegin(); p != itemList.rend(); ++p){
        if(p->item.itemID == itemID){
            itemList.erase(std::next(p).base());
            if(post){
                postGridItemIDList(x, y);
            }
            return;
        }
    }
}

SDGroundItemIDList ServerMap::getGroundItemIDList(int x, int y, size_t r)
{
    fflassert(r > 0);

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
    fflassert(groundValid(x, y));
    const auto sdBuf = cerealf::serialize(getGroundItemIDList(x, y, 1), true);

    doCircle(x, y, 20, [&sdBuf, this](int x, int y) -> bool
    {
        if(groundValid(x, y)){
            doUIDList(x, y, [&sdBuf, this](uint64_t uid) -> bool
            {
                if(uidf::getUIDType(uid) == UID_PLY){
                    forwardNetPackage(uid, SM_GROUNDITEMIDLIST, sdBuf);
                }
                return false;
            });
        }
        return false;
    });
}

void ServerMap::postGroundItemIDList(uint64_t uid, int x, int y)
{
    fflassert(groundValid(x, y));
    fflassert(uidf::getUIDType(uid) == UID_PLY);
    forwardNetPackage(uid, SM_GROUNDITEMIDLIST, cerealf::serialize(getGroundItemIDList(x, y, 20)));
}

SDGroundFireWallList ServerMap::getGroundFireWallList(int x, int y, size_t r)
{
    fflassert(r > 0);

    // center (x, y) can be invalid
    // an invalid center can cover valid grids

    SDGroundFireWallList groundFireWallList;
    groundFireWallList.mapID = ID();

    doCircle(x, y, r, [&groundFireWallList, this](int x, int y) -> bool
    {
        if(groundValid(x, y)){
            for(auto p = getGrid(x, y).fireWallList.begin(); p != getGrid(x, y).fireWallList.end();){
                if(hres_tstamp().to_msec() >= p->startTime + p->duration){
                    p = getGrid(x, y).fireWallList.erase(p);
                }
                else{
                    p++;
                }
            }

            groundFireWallList.fireWallList.push_back(SDGridFireWall
            {
                .x = x,
                .y = y,
                .count = to_d(getGrid(x, y).fireWallList.size()),
            });
        }
        return false;
    });
    return groundFireWallList;
}

void ServerMap::postGridFireWallList(int x, int y)
{
    fflassert(groundValid(x, y));
    const auto sdBuf = cerealf::serialize(getGroundFireWallList(x, y, 1), true);

    doCircle(x, y, 20, [&sdBuf, this](int x, int y) -> bool
    {
        if(groundValid(x, y)){
            doUIDList(x, y, [&sdBuf, this](uint64_t uid) -> bool
            {
                if(uidf::getUIDType(uid) == UID_PLY){
                    forwardNetPackage(uid, SM_GROUNDFIREWALLLIST, sdBuf);
                }
                return false;
            });
        }
        return false;
    });
}

void ServerMap::postGroundFireWallList(uint64_t uid, int x, int y)
{
    fflassert(groundValid(x, y));
    fflassert(uidf::getUIDType(uid) == UID_PLY);
    forwardNetPackage(uid, SM_GROUNDFIREWALLLIST, cerealf::serialize(getGroundFireWallList(x, y, 20)));
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

    if(const auto loc = GetValidGrid(false, false, to_d(bStrictLoc), nHintX, nHintY); loc.has_value()){
        const auto [nDstX, nDstY] = loc.value();
        Monster *monsterPtr = nullptr;
        switch(nMonsterID){
            case DBCOM_MONSTERID(u8"变异骷髅"):
                {
                    monsterPtr = new TaoSkeleton
                    {
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
                        this,
                        nDstX,
                        nDstY,
                        DIR_UP, // TODO face its master
                        nMasterUID,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"食人花"):
                {
                    monsterPtr = new CannibalPlant
                    {
                        this,
                        nDstX,
                        nDstY,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"角蝇"):
                {
                    monsterPtr = new BugbatMaggot
                    {
                        this,
                        nDstX,
                        nDstY,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"蝙蝠"):
                {
                    monsterPtr = new Monster
                    {
                        nMonsterID,
                        this,
                        nDstX,
                        nDstY,
                        DIR_LEFT, // direction for initial gfx when born
                        nMasterUID,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"栗子树"):
            case DBCOM_MONSTERID(u8"圣诞树"):
                {
                    monsterPtr = new MonsterTree
                    {
                        nMonsterID,
                        this,
                        nDstX,
                        nDstY,
                    };
                    break;
                }
            default:
                {
                    monsterPtr = new Monster
                    {
                        nMonsterID,
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

    if(g_serverArgParser->disableGuardSpawn){
        return nullptr;
    }

    auto guardPtr = new Guard
    {
        monID,
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
    if(g_serverArgParser->disableNPCSpawn){
        return nullptr;
    }

    try{
        auto npcPtr = new NPChar(this, std::make_unique<NPChar::LuaNPCModule>(initParam));
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

    if(const auto loc = GetValidGrid(false, false, to_d(bStrictLoc), nHintX, nHintY); loc.has_value()){
        auto playerPtr = new Player
        {
            initPlayer,
            this,
        };

        playerPtr->activate();
        return playerPtr;
    }
    return nullptr;
}

int ServerMap::CheckPathGrid(int nX, int nY) const
{
    if(!m_mir2xMapData.validC(nX, nY)){
        return PathFind::INVALID;
    }

    if(!m_mir2xMapData.cell(nX, nY).land.canThrough()){
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

void ServerMap::updateMapGrid()
{
    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){

            bool hasDoneWallFire = false;
            auto &gridFireWallList = getGrid(nX, nY).fireWallList;

            for(auto p = gridFireWallList.begin(); p != gridFireWallList.end();){
                const auto currTime = hres_tstamp().to_msec();
                if(currTime >= p->startTime + p->duration){
                    hasDoneWallFire = true;
                    p = gridFireWallList.erase(p);
                    continue;
                }

                if(p->dps > 0 && currTime > p->lastAttackTime + 1000 / p->dps){
                    AMAttack amA;
                    std::memset(&amA, 0, sizeof(amA));

                    amA.UID = p->uid;
                    amA.mapID = UID();
                    amA.X = nX;
                    amA.Y = nY;

                    amA.damage = DamageNode
                    {
                        .magicID = to_d(DBCOM_MAGICID(u8"火墙")),
                        .damage  = mathf::rand(p->minDC, p->maxDC),
                        .effect  = 0,
                    };

                    doUIDList(nX, nY, [amA, this](uint64_t uid) -> bool
                    {
                        if(uid != amA.UID){
                            switch(uidf::getUIDType(uid)){
                                case UID_PLY:
                                case UID_MON:
                                    {
                                        m_actorPod->forward(uid, {AM_ATTACK, amA});
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                        }
                        return false;
                    });
                    p->lastAttackTime = currTime;
                }
                p++;
            }

            if(hasDoneWallFire){
                postGridFireWallList(nX, nY);
            }

            bool hasItemExpired = false;
            auto &gridItemList = getGridItemList(nX, nY);

            for(auto p = gridItemList.begin(); p != gridItemList.end();){
                if(hres_tstamp().to_msec() >= p->dropTime + 120 * 1000){
                    hasItemExpired = true;
                    p = gridItemList.erase(p);
                }
                else{
                    p++;
                }
            }

            if(hasItemExpired){
                postGridItemIDList(nX, nY);
            }
        }
    }
}

void ServerMap::loadNPChar()
{
    const auto cfgScriptPath = g_serverConfigureWindow->getConfig().scriptPath;
    const auto scriptPath = cfgScriptPath.empty() ? std::string("script/npc") : (cfgScriptPath + "/npc");

    const auto reg = str_printf("%s\\..*\\.lua", to_cstr(DBCOM_MAPRECORD(ID()).name));
    for(const auto &fileName: filesys::getFileList(scriptPath.c_str(), true, reg.c_str())){
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
