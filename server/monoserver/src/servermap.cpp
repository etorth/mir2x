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

ServerMap::ServerMapLuaModule::ServerMapLuaModule(ServerMap *mapPtr)
{
    if(!mapPtr){
        throw fflerror("ServerMapLuaModule binds to empty ServerMap");
    }

    getLuaState().set_function("scriptDone", []() -> bool
    {
        return false;
    });

    getLuaState().set_function("getMapID", [mapPtr]() -> int
    {
        return (int)(mapPtr->ID());
    });

    getLuaState().set_function("getMapName", [mapPtr]() -> std::string
    {
        return std::string(to_cstr(DBCOM_MAPRECORD(mapPtr->ID()).name));
    });

    getLuaState().set_function("getMapSize", [mapPtr]()
    {
        return sol::as_returns(std::vector<int>{mapPtr->W(), mapPtr->H()});
    });

    getLuaState().set_function("getCanThroughGridCount", [mapPtr, gridCount = (int)(-1)]() mutable -> int
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

    getLuaState().set_function("getMonsterList", [mapPtr](sol::this_state thisLua)
    {
        // convert to std::string
        // sol don't support std::u8string for now
        std::vector<std::string> monNameList;
        const auto monU8NameList = mapPtr->getMonsterList();

        monNameList.reserve(monU8NameList.size());
        for(const auto &monName: monU8NameList){
            monNameList.push_back(to_cstr(monName));
        }
        return sol::make_object(sol::state_view(thisLua), monNameList);
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
                    return mapPtr->GetMonsterCount(0);
                }
            case 1:
                {
                    if(argList[0].is<int>()){
                        if(const int monID = argList[0].as<int>(); monID >= 0){
                            return mapPtr->GetMonsterCount(monID);
                        }
                    }

                    else if(argList[0].is<std::string>()){
                        if(const int monID = DBCOM_MONSTERID(to_u8cstr(argList[0].as<std::string>().c_str())); monID >= 0){
                            return mapPtr->GetMonsterCount(monID);
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

    getLuaState().set_function("addNPC", [mapPtr](int npcID, sol::variadic_args args) -> bool
    {
        const std::vector<sol::object> argList(args.begin(), args.end());
        switch(argList.size()){
            case 0:
                {
                    return mapPtr->addNPChar((uint16_t)(npcID), -1, -1, 0, false);
                }
            case 2:
                {
                    if(argList[0].is<int>() && argList[1].is<int>()){
                        return mapPtr->addNPChar((uint32_t)(npcID), argList[0].as<int>(), argList[1].as<int>(), 0, false);
                    }
                    break;
                }
            case 3:
                {
                    if(argList[0].is<int>() && argList[1].is<int>() && argList[2].is<bool>()){
                        return mapPtr->addNPChar((uint32_t)(npcID), argList[0].as<int>(), argList[1].as<int>(), 0, argList[2].as<bool>());
                    }
                    break;
                }
            case 4:
                {
                    if(argList[0].is<int>() && argList[1].is<int>() && argList[2].is<int>() && argList[3].is<bool>()){
                        return mapPtr->addNPChar((uint32_t)(npcID), argList[0].as<int>(), argList[1].as<int>(), argList[2].is<int>(), argList[3].as<bool>());
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

    getLuaState().script_file([mapPtr]() -> std::string
    {
        const auto configScriptPath = g_serverConfigureWindow->getScriptPath();
        const auto scriptPath = configScriptPath.empty() ? std::string("script/map") : configScriptPath;

        const auto scriptName = str_printf("%s/%s.lua", scriptPath.c_str(), to_cstr(DBCOM_MAPRECORD(mapPtr->ID()).name));
        if(std::filesystem::exists(scriptName)){
            return scriptName;
        }

        const auto defaultScriptName = scriptPath + "/default.lua";
        if(std::filesystem::exists(defaultScriptName)){
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

    m_cellVec2D.resize(W());
    m_cellVec2D.shrink_to_fit();

    for(auto &rstStateLine: m_cellVec2D){
        rstStateLine.resize(H());
        rstStateLine.shrink_to_fit();
    }

    for(const auto &entry: DBCOM_MAPRECORD(nMapID).linkArray){
        if(true
                && entry.w > 0
                && entry.h > 0
                && ValidC(entry.x, entry.y)){

            for(int nW = 0; nW < entry.w; ++nW){
                for(int nH = 0; nH < entry.h; ++nH){
                    getCell(entry.x + nW, entry.y + nH).mapID   = DBCOM_MAPID(entry.endName);
                    getCell(entry.x + nW, entry.y + nH).switchX = entry.endX;
                    getCell(entry.x + nW, entry.y + nH).switchY = entry.endY;
                }
            }
        }else{
            break;
        }
    }
}

void ServerMap::operateAM(const MessagePack &rstMPK)
{
    switch(rstMPK.Type()){
        case MPK_PICKUP:
            {
                on_MPK_PICKUP(rstMPK);
                break;
            }
        case MPK_NEWDROPITEM:
            {
                on_MPK_NEWDROPITEM(rstMPK);
                break;
            }
        case MPK_TRYLEAVE:
            {
                on_MPK_TRYLEAVE(rstMPK);
                break;
            }
        case MPK_UPDATEHP:
            {
                on_MPK_UPDATEHP(rstMPK);
                break;
            }
        case MPK_DEADFADEOUT:
            {
                on_MPK_DEADFADEOUT(rstMPK);
                break;
            }
        case MPK_ACTION:
            {
                on_MPK_ACTION(rstMPK);
                break;
            }
        case MPK_BADACTORPOD:
            {
                on_MPK_BADACTORPOD(rstMPK);
                break;
            }
        case MPK_TRYMOVE:
            {
                on_MPK_TRYMOVE(rstMPK);
                break;
            }
        case MPK_PATHFIND:
            {
                on_MPK_PATHFIND(rstMPK);
                break;
            }
        case MPK_TRYMAPSWITCH:
            {
                on_MPK_TRYMAPSWITCH(rstMPK);
                break;
            }
        case MPK_METRONOME:
            {
                on_MPK_METRONOME(rstMPK);
                break;
            }
        case MPK_TRYSPACEMOVE:
            {
                on_MPK_TRYSPACEMOVE(rstMPK);
                break;
            }
        case MPK_ADDCHAROBJECT:
            {
                on_MPK_ADDCHAROBJECT(rstMPK);
                break;
            }
        case MPK_PULLCOINFO:
            {
                on_MPK_PULLCOINFO(rstMPK);
                break;
            }
        case MPK_QUERYCOCOUNT:
            {
                on_MPK_QUERYCOCOUNT(rstMPK);
                break;
            }
        case MPK_QUERYRECTUIDLIST:
            {
                on_MPK_QUERYRECTUIDLIST(rstMPK);
                break;
            }
        case MPK_OFFLINE:
            {
                on_MPK_OFFLINE(rstMPK);
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
            for(auto nUID: getUIDList(nX, nY)){
                if(auto nType = uidf::getUIDType(nUID); nType == UID_PLY || nType == UID_MON){
                    return false;
                }
            }
        }

        if(bCheckLock){
            if(getCell(nX, nY).Locked){
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

uint64_t ServerMap::activate()
{
    const auto uid = ServerObject::activate();
    if(!uid){
        return 0;
    }

    if(m_luaModulePtr){
        throw fflerror("ServerMap lua module has been loaded twice: %s", to_cstr(DBCOM_MAPRECORD(ID()).name));
    }

    m_luaModulePtr = std::make_unique<ServerMap::ServerMapLuaModule>(this);
    return uid;
}

void ServerMap::addGridUID(uint64_t uid, int nX, int nY, bool bForce)
{
    if(!ValidC(nX, nY)){
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
    if(!ValidC(nX, nY)){
        throw fflerror("invalid location: (%d, %d)", nX, nY);
    }

    const auto &uidList = getUIDList(nX, nY);
    return std::find(uidList.begin(), uidList.end(), uid) != uidList.end();
}

void ServerMap::removeGridUID(uint64_t uid, int nX, int nY)
{
    if(!ValidC(nX, nY)){
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
                for(auto nUID: getUIDList(nX, nY)){
                    if(uidf::getUIDType(nUID) == UID_PLY){
                        m_actorPod->forward(nUID, {MPK_SHOWDROPITEM, stAMSDI});
                    }
                }
            }
            return false;
        };

        doCircle(nX, nY, 10, fnNotifyDropItem);
        return true;
    }
    return false;
}

int ServerMap::GetMonsterCount(uint32_t nMonsterID)
{
    int nCount = 0;
    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){
            for(auto nUID: getUIDList(nX, nY)){
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

std::vector<std::u8string> ServerMap::getMonsterList() const
{
    return
    {
        u8"虎卫",
        u8"沙漠石人",
        u8"红蛇",
        u8"虎蛇",
    };
}

void ServerMap::notifyNewCO(uint64_t nUID, int nX, int nY)
{
    AMNotifyNewCO stAMNNCO;
    std::memset(&stAMNNCO, 0, sizeof(stAMNNCO));

    stAMNNCO.UID = nUID;
    doCircle(nX, nY, 20, [this, stAMNNCO](int nX, int nY) -> bool
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

Monster *ServerMap::addMonster(uint32_t nMonsterID, uint64_t nMasterUID, int nHintX, int nHintY, bool bStrictLoc)
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

        pMonster->activate();

        addGridUID (pMonster->UID(), nDstX, nDstY, true);
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

        pNPC->activate();

        addGridUID (pNPC->UID(), dstX, dstY, true);
        notifyNewCO(pNPC->UID(), dstX, dstY);

        return pNPC;
    }
    return nullptr;
}

Player *ServerMap::addPlayer(uint32_t nDBID, int nHintX, int nHintY, int nDirection, bool bStrictLoc)
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

        pPlayer->activate();

        addGridUID (pPlayer->UID(), nDstX, nDstY, true);
        notifyNewCO(pPlayer->UID(), nDstX, nDstY);

        return pPlayer;
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

    if(getCell(nX, nY).Locked){
        return PathFind::LOCKED;
    }

    return PathFind::FREE;
}
