#include <regex>
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
#include "filesys.hpp"
#include "raiitimer.hpp"
#include "mathf.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "mapbindb.hpp"
#include "servermap.hpp"
#include "charobject.hpp"
#include "monoserver.hpp"
#include "rotatecoord.hpp"
#include "serverargparser.hpp"
#include "serverconfigurewindow.hpp"
#include "serverguard.hpp"
#include "servertaodog.hpp"
#include "servertaoskeleton.hpp"
#include "servertaoskeletonext.hpp"
#include "servercannibalplant.hpp"
#include "serverbugbatmaggot.hpp"
#include "servermonstertree.hpp"
#include "serverdualaxeskeleton.hpp"
#include "servereviltentacle.hpp"
#include "serversandcactus.hpp"
#include "serversandghost.hpp"
#include "serverrebornzombie.hpp"
#include "serveranthealer.hpp"
#include "serverwoomataurus.hpp"
#include "serverevilcentipede.hpp"
#include "serverzumamonster.hpp"
#include "serverzumataurus.hpp"
#include "serverbombspider.hpp"
#include "serverrootspider.hpp"
#include "serverredmoonevil.hpp"
#include "servershipwrecklord.hpp"
#include "serverminotaurguardian.hpp"

extern MapBinDB *g_mapBinDB;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;
extern ServerConfigureWindow *g_serverConfigureWindow;

ServerMap::LuaThreadRunner::LuaThreadRunner(ServerMap *serverMapPtr)
    : ServerObject::LuaThreadRunner(serverMapPtr)
{
    bindFunction("getMapID", [this]() -> int
    {
        return to_d(getServerMap()->ID());
    });

    bindFunction("getMapName", [this]() -> std::string
    {
        return std::string(to_cstr(DBCOM_MAPRECORD(getServerMap()->ID()).name));
    });

    bindFunction("getMapSize", [this]()
    {
        return sol::as_returns(std::vector<int>{getServerMap()->W(), getServerMap()->H()});
    });

    bindFunction("getCanThroughGridCount", [this, gridCount = to_d(-1)]() mutable -> int
    {
        if(gridCount >= 0){
            return gridCount;
        }

        gridCount = 0;
        for(int x = 0; x < getServerMap()->W(); ++x){
            for(int y = 0; y < getServerMap()->H(); ++y){
                if(getServerMap()->groundValid(x, y)){
                    gridCount++;
                }
            }
        }
        return gridCount;
    });

    bindFunction("getRandLoc", [this]()
    {
        std::array<int, 2> loc;
        while(true){
            const int x = std::rand() % getServerMap()->W();
            const int y = std::rand() % getServerMap()->H();

            if(getServerMap()->groundValid(x, y)){
                loc[0] = x;
                loc[1] = y;
                break;
            }
        }
        return sol::as_returns(loc);
    });

    bindFunction("countGLoc", [fullCount = -1, this](sol::variadic_args args) mutable
    {
        const std::vector<sol::object> argList(args.begin(), args.end());
        const auto [useFullMap, regionX, regionY, regionW, regionH] = [&argList, this]() -> std::tuple<bool, int, int, int, int>
        {
            switch(argList.size()){
                case 0:
                    {
                        return
                        {
                            true,
                            0,
                            0,
                            getServerMap()->W(),
                            getServerMap()->H(),
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

        if(!mathf::rectangleOverlapRegion<int>(0, 0, getServerMap()->W(), getServerMap()->H(), roiX, roiY, roiW, roiH)){
            throw fflerror("invalid region: map = %s, x = %d, y = %d, w = %d, h = %d", to_cstr(DBCOM_MAPRECORD(getServerMap()->ID()).name), regionX, regionY, regionW, regionH);
        }

        int count = 0;
        for(int yi = roiY; yi < roiY + roiH; ++yi){
            for(int xi = roiX; xi < roiX + roiW; ++xi){
                if(getServerMap()->groundValid(xi, yi)){
                    count++;
                }
            }
        }

        if(useFullMap){
            fullCount = count; // cache
        }
        return count;
    });

    bindFunction("randGLoc", [this](sol::variadic_args args)
    {
        const std::vector<sol::object> argList(args.begin(), args.end());
        const auto [useFullMap, regionX, regionY, regionW, regionH] = [&argList, this]() -> std::tuple<bool, int, int, int, int>
        {
            switch(argList.size()){
                case 0:
                    {
                        return
                        {
                            true,
                            0,
                            0,
                            getServerMap()->W(),
                            getServerMap()->H(),
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

        // randGLoc doesn't check CO's on map, only check groundValid()
        // given region should have been checked by countGLoc to make sure it has valid grid

        // give a maximal loop count
        // script may give bad region without checking

        constexpr int maxTryCount = 1024;
        for(int i = 0; i < maxTryCount; ++i){
            if(const auto locopt = getServerMap()->getRCValidGrid(false, false, 1, regionX, regionY, regionW, regionH); locopt.has_value()){
                return sol::as_returns(std::array<int, 2>
                {
                    std::get<0>(locopt.value()),
                    std::get<1>(locopt.value()),
                });
            }
        }

        // failed to pick a random location
        // try brutle force to get a location, shall always succeeds if given region has valid grid(s)

        if(const auto locopt = getServerMap()->getRCValidGrid(false, false, -1, regionX, regionY, regionW, regionH); locopt.has_value()){
            return sol::as_returns(std::array<int, 2>
            {
                std::get<0>(locopt.value()),
                std::get<1>(locopt.value()),
            });
        }

        // give detailed failure message
        // need it to validate map monster gen coroutine, otherwise this can throw

        if(useFullMap){
            throw fflerror("no valid grid on map: map = %s", to_cstr(DBCOM_MAPRECORD(getServerMap()->ID()).name));
        }
        else{
            throw fflerror("no valid grid in region: map = %s, x = %d, y = %d, w = %d, h = %d", to_cstr(DBCOM_MAPRECORD(getServerMap()->ID()).name), regionX, regionY, regionW, regionH);
        }
    });

    bindFunction("getNPCharUID", [this](std::string npcName, sol::this_state s) -> sol::object
    {
        for(const auto [uid, npcPtr]: getServerMap()->m_npcList){
            if(npcPtr->getNPCName() == npcName){
                return luaf::buildLuaObj(sol::state_view(s), lua_Integer(uid));
            }
        }
        return luaf::buildLuaObj(sol::state_view(s), luaf::luaNil{});
    });

    bindFunction("getMonsterCount", [this](sol::variadic_args args) -> int
    {
        const std::vector<sol::object> argList(args.begin(), args.end());
        switch(argList.size()){
            case 0:
                {
                    return getServerMap()->getMonsterCount(0);
                }
            case 1:
                {
                    if(argList[0].is<int>()){
                        if(const int monID = argList[0].as<int>(); monID >= 0){
                            return getServerMap()->getMonsterCount(monID);
                        }
                    }

                    else if(argList[0].is<std::string>()){
                        if(const int monID = DBCOM_MONSTERID(to_u8cstr(argList[0].as<std::string>().c_str())); monID >= 0){
                            return getServerMap()->getMonsterCount(monID);
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

    bindFunction("addMonster", [this](sol::object monInfo, sol::variadic_args args) -> uint64_t
    {
        const auto fnGetMonsterUID = [](const Monster *monPtr) -> uint64_t
        {
            return monPtr ? monPtr->UID() : 0;
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

        if(monID){
            const std::vector<sol::object> argList(args.begin(), args.end());
            switch(argList.size()){
                case 0:
                    {
                        return fnGetMonsterUID(getServerMap()->addMonster(monID, 0, -1, -1, false));
                    }
                case 2:
                    {
                        if(true
                                && argList[0].is<int>()
                                && argList[1].is<int>()){

                            const auto nX = argList[0].as<int>();
                            const auto nY = argList[1].as<int>();
                            return fnGetMonsterUID(getServerMap()->addMonster(monID, 0, nX, nY, false));
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
                            return fnGetMonsterUID(getServerMap()->addMonster(monID, 0, nX, nY, bStrictLoc));
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }
        return fnGetMonsterUID(nullptr);
    });

    bindFunction("addGuard", [this](std::string type, int x, int y, int direction) -> bool
    {
        const uint32_t monID = DBCOM_MONSTERID(to_u8cstr(type));
        if(!monID){
            return false;
        }
        return getServerMap()->addGuard(monID, x, y, direction);
    });

    pfrCheck(execFile([this]() -> std::string
    {
        const auto configScriptPath = g_serverConfigureWindow->getConfig().scriptPath;
        const auto scriptPath = configScriptPath.empty() ? std::string("script/map") : (configScriptPath + "/map");

        const auto scriptName = str_printf("%s/%s.lua", scriptPath.c_str(), to_cstr(DBCOM_MAPRECORD(getServerMap()->ID()).name));
        if(filesys::hasFile(scriptName.c_str())){
            return scriptName;
        }

        const auto defaultScriptName = scriptPath + "/default.lua";
        if(filesys::hasFile(defaultScriptName.c_str())){
            return defaultScriptName;
        }
        throw fflerror("can't load proper script for map %s", to_cstr(DBCOM_MAPRECORD(getServerMap()->ID()).name));
    }().c_str()));

    pfrCheck(execRawString(BEGIN_LUAINC(char)
#include "servermap.lua"
    END_LUAINC()));
}

ServerMap::ServerPathFinder::ServerPathFinder(const ServerMap *mapPtr, int argMaxStep, int argCheckCO)
    : AStarPathFinder(0, argMaxStep, [this](int srcX, int srcY, int srcDir, int dstX, int dstY) -> std::optional<double>
      {
          // treat checkCO and checkLock same
          // from server's view occupied and to-be-occupied are equivlent
          fflassert(pathf::hopValid(maxStep(), srcX, srcY, dstX, dstY));
          return m_map->oneStepCost(m_checkCO, m_checkCO, srcX, srcY, srcDir, dstX, dstY);
      })

    , m_map(mapPtr)
    , m_checkCO(argCheckCO)
{
    fflassert(m_map);

    fflassert(m_checkCO >= 0, m_checkCO);
    fflassert(m_checkCO <= 2, m_checkCO);

    fflassert(maxStep() >= 1, maxStep());
    fflassert(maxStep() <= 3, maxStep());
}

ServerMap::ServerMap(uint32_t mapID)
    : ServerObject(uidf::getMapBaseUID(mapID))
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
        }
        else{
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
        case AM_REMOTECALL:
            {
                on_AM_REMOTECALL(rstMPK);
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

std::optional<double> ServerMap::oneStepCost(int checkCO, int checkLock, int srcX, int srcY, int srcDir, int dstX, int dstY) const
{
    fflassert(checkCO >= 0, checkCO);
    fflassert(checkCO <= 2, checkCO);

    fflassert(checkLock >= 0, checkLock);
    fflassert(checkLock <= 2, checkLock);

    int hopSize = -1;
    switch(mathf::LDistance2(srcX, srcY, dstX, dstY)){
        case  1:
        case  2: hopSize = 1; break;
        case  4:
        case  8: hopSize = 2; break;
        case  9:
        case 18: hopSize = 3; break;
        case  0: return .0;
        default: return {};
    }

    double gridExtraPen = 0.00;
    const auto hopDir = pathf::getOffDir(srcX, srcY, dstX, dstY);

    for(int stepSize = 1; stepSize <= hopSize; ++stepSize){
        const auto [currX, currY] = pathf::getFrontGLoc(srcX, srcY, hopDir, stepSize);
        switch(const auto pfGrid = checkPathGrid(currX, currY)){
            case PF_FREE:
                {
                    break;
                }
            case PF_OCCUPIED:
                {
                    switch(checkCO){
                        case 0:
                            {
                                break;
                            }
                        case 1:
                            {
                                gridExtraPen += 100.00;
                                break;
                            }
                        case 2:
                            {
                                return {};
                            }
                        default:
                            {
                                throw fflvalue(checkCO);
                            }
                    }
                    break;
                }
            case PF_LOCKED:
                {
                    if(stepSize == hopSize){
                        switch(checkLock){
                            case 0:
                                {
                                    break;
                                }
                            case 1:
                                {
                                    gridExtraPen += 100.00;
                                    break;
                                }
                            case 2:
                                {
                                    return {};
                                }
                            default:
                                {
                                    throw fflvalue(checkLock);
                                }
                        }
                    }
                    break;
                }
            case PF_NONE:
            case PF_OBSTACLE:
                {
                    return {};
                }
            default:
                {
                    throw fflvalue(currX, currY, pfGrid);
                }
        }
    }

    return 1.00 + hopSize * 0.10 + gridExtraPen + pathf::getDirAbsDiff(srcDir, hopDir) * 0.01;
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

        if(in(ID(), currX, currY) && canMove(checkCO, checkLock, currX, currY)){
            return std::make_tuple(currX, currY);
        }

        if(!rc.forward()){
            break;
        }
    }
    return {};
}

std::optional<std::tuple<int, int>> ServerMap::getRCValidGrid(bool checkCO, bool checkLock, int checkCount) const
{
    return getRCValidGrid(checkCO, checkLock, checkCount, 0, 0, W(), H());
}

std::optional<std::tuple<int, int>> ServerMap::getRCValidGrid(bool checkCO, bool checkLock, int checkCount, int startX, int startY) const
{
    return getRCGLoc(checkCO, checkLock, checkCount, startX, startY, 0, 0, W(), H());
}

std::optional<std::tuple<int, int>> ServerMap::getRCValidGrid(bool checkCO, bool checkLock, int checkCount, int regionX, int regionY, int regionW, int regionH) const
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

    m_gridItemLocList.insert({x, y});
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

    m_gridItemLocList.insert({x, y});
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

    if(const auto loc = getRCValidGrid(false, false, to_d(bStrictLoc), nHintX, nHintY); loc.has_value()){
        const auto [nDstX, nDstY] = loc.value();
        Monster *monsterPtr = nullptr;
        switch(nMonsterID){
            case DBCOM_MONSTERID(u8"变异骷髅"):
                {
                    monsterPtr = new ServerTaoSkeleton
                    {
                        this,
                        nDstX,
                        nDstY,
                        nMasterUID,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"超强骷髅"):
                {
                    monsterPtr = new ServerTaoSkeletonExt
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
                    monsterPtr = new ServerTaoDog
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
                    monsterPtr = new ServerCannibalPlant
                    {
                        this,
                        nDstX,
                        nDstY,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"角蝇"):
                {
                    monsterPtr = new ServerBugbatMaggot
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
            case DBCOM_MONSTERID(u8"圣诞树1"):
                {
                    monsterPtr = new ServerMonsterTree
                    {
                        nMonsterID,
                        this,
                        nDstX,
                        nDstY,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"沙漠树魔"):
                {
                    monsterPtr = new ServerSandCactus
                    {
                        this,
                        nDstX,
                        nDstY,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"掷斧骷髅"):
                {
                    monsterPtr = new ServerDualAxeSkeleton
                    {
                        this,
                        nDstX,
                        nDstY,
                        DIR_UP,
                        nMasterUID,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"触角神魔"):
            case DBCOM_MONSTERID(u8"爆毒神魔"):
                {
                    monsterPtr = new ServerEvilTentacle
                    {
                        nMonsterID,
                        this,
                        nDstX,
                        nDstY,
                        DIR_UP,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"沙鬼"):
                {
                    monsterPtr = new ServerSandGhost
                    {
                        this,
                        nDstX,
                        nDstY,
                        DIR_UP,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"僵尸_1"):
            case DBCOM_MONSTERID(u8"僵尸_2"):
            case DBCOM_MONSTERID(u8"腐僵"):
                {
                    monsterPtr = new ServerRebornZombie
                    {
                        nMonsterID,
                        this,
                        nDstX,
                        nDstY,
                        DIR_UP,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"蚂蚁道士"):
                {
                    monsterPtr = new ServerAntHealer
                    {
                        this,
                        nDstX,
                        nDstY,
                        DIR_UP,
                        nMasterUID,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"沃玛教主"):
                {
                    monsterPtr = new ServerWoomaTaurus
                    {
                        this,
                        nDstX,
                        nDstY,
                        DIR_UP,
                        nMasterUID,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"触龙神"):
                {
                    monsterPtr = new ServerEvilCentipede
                    {
                        this,
                        nDstX,
                        nDstY,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"祖玛雕像"):
            case DBCOM_MONSTERID(u8"祖玛卫士"):
                {
                    monsterPtr = new ServerZumaMonster
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
            case DBCOM_MONSTERID(u8"祖玛教主"):
                {
                    monsterPtr = new ServerZumaTaurus
                    {
                        this,
                        nDstX,
                        nDstY,
                        nMasterUID,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"爆裂蜘蛛"):
                {
                    monsterPtr = new ServerBombSpider
                    {
                        this,
                        nDstX,
                        nDstY,
                        DIR_UP,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"幻影蜘蛛"):
                {
                    monsterPtr = new ServerRootSpider
                    {
                        this,
                        nDstX,
                        nDstY,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"赤月恶魔"):
                {
                    monsterPtr = new ServerRedMoonEvil
                    {
                        this,
                        nDstX,
                        nDstY,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"霸王教主"):
                {
                    monsterPtr = new ServerShipwreckLord
                    {
                        this,
                        nDstX,
                        nDstY,
                        DIR_UP,
                        nMasterUID,
                    };
                    break;
                }
            case DBCOM_MONSTERID(u8"潘夜左护卫"):
            case DBCOM_MONSTERID(u8"潘夜右护卫"):
                {
                    monsterPtr = new ServerMinotaurGuardian
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

ServerGuard *ServerMap::addGuard(uint32_t monID, int x, int y, int direction)
{
    fflassert(monID);
    fflassert(validC(x, y));

    if(g_serverArgParser->disableGuardSpawn){
        return nullptr;
    }

    auto guardPtr = new ServerGuard
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
        auto npcPtr = new NPChar(this, initParam);
        m_npcList[npcPtr->UID()] = npcPtr;
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

    if(const auto loc = getRCValidGrid(false, false, to_d(bStrictLoc), nHintX, nHintY); loc.has_value()){
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

int ServerMap::checkPathGrid(int argX, int argY) const
{
    if(!m_mir2xMapData.validC(argX, argY)){
        return PF_NONE;
    }

    if(!m_mir2xMapData.cell(argX, argY).land.canThrough()){
        return PF_OBSTACLE;
    }

    if(!getUIDList(argX, argY).empty()){
        return PF_OCCUPIED;
    }

    if(getGrid(argX, argY).locked){
        return PF_LOCKED;
    }

    return PF_FREE;
}

void ServerMap::updateMapGrid()
{
    updateMapGridFireWall();
    updateMapGridGroundItem();
}

void ServerMap::updateMapGridFireWall()
{
    for(auto locIter = m_fireWallLocList.begin(); locIter != m_fireWallLocList.end();){
        const auto nX = std::get<0>(*locIter);
        const auto nY = std::get<1>(*locIter);

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

                amA.damage = MagicDamage
                {
                    .magicID = to_d(DBCOM_MAGICID(u8"火墙")),
                    .damage = mathf::rand(p->minDC, p->maxDC),
                    .mcHit = p->mcHit,
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

        if(gridFireWallList.empty()){
            locIter = m_fireWallLocList.erase(locIter);
        }
        else{
            locIter++;
        }
    }
}

void ServerMap::updateMapGridGroundItem()
{
    for(auto locIter = m_gridItemLocList.begin(); locIter != m_gridItemLocList.end();){
        const auto nX = std::get<0>(*locIter);
        const auto nY = std::get<1>(*locIter);

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

        if(gridItemList.empty()){
            locIter = m_gridItemLocList.erase(locIter);
        }
        else{
            locIter++;
        }
    }
}

void ServerMap::onActivate()
{
    ServerObject::onActivate();
    loadNPChar();

    m_luaRunner = std::make_unique<ServerMap::LuaThreadRunner>(this);
    m_luaRunner->spawn(m_mainScriptThreadKey, "return main()");
}

void ServerMap::loadNPChar()
{
    const auto cfgScriptPath = g_serverConfigureWindow->getConfig().scriptPath;
    const auto scriptPath = cfgScriptPath.empty() ? std::string("script/npc") : (cfgScriptPath + "/npc");

    // npc script file has format:
    // <MAP名称>.GLOC_x_y_dir.<NPC名称>.LOOK_id.lua

    const auto expr = str_printf(R"#(%s\.(.*)\.GLOC_(\d+)_(\d+)_(\d+)\.LOOK_(\d+)\.lua)#", to_cstr(DBCOM_MAPRECORD(ID()).name));
    //                               --  ----       ----- -----  ----       -----
    //                               ^    ^           ^     ^     ^           ^
    //                               |    |           |     |     |           |
    //                               |    |           |     |     |           +------- look id
    //                               |    |           |     |     +------------------- npc stand gfx dir (may not be 8-dir)
    //                               |    |           |     +------------------------- npc grid y
    //                               |    |           +------------------------------- npc grid x
    //                               |    +------------------------------------------- npc name
    //                               +------------------------------------------------ map name
    const std::regex regExpr(expr);
    for(const auto &fileName: filesys::getFileList(scriptPath.c_str(), false, expr.c_str())){
        std::match_results<std::string::const_iterator> result;
        if(std::regex_match(fileName.begin(), fileName.end(), result, regExpr)){
            SDInitNPChar initNPChar
            {
                .fullScriptName = scriptPath + "/" + fileName,
                .mapID = ID(),
            };

            for(int i = 0; const auto &m: result){
                switch(i++){
                    case 1 : initNPChar.npcName =           m.str() ; break;
                    case 2 : initNPChar.x       = std::stoi(m.str()); break;
                    case 3 : initNPChar.y       = std::stoi(m.str()); break;
                    case 4 : initNPChar.gfxDir  = std::stoi(m.str()); break;
                    case 5 : initNPChar.lookID  = std::stoi(m.str()); break;
                    default:                                        ; break;
                }
            }

            addNPChar(initNPChar);
        }
        else{
            throw fflvalue(fileName);
        }
    }
}
