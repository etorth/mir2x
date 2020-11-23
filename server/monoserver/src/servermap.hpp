/*
 * =====================================================================================
 *
 *       Filename: servermap.hpp
 *        Created: 09/03/2015 03:49:00
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

#pragma once

#include <tuple>
#include <memory>
#include <vector>
#include <cstdint>
#include <concepts>

#include "mathf.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "querytype.hpp"
#include "commonitem.hpp"
#include "pathfinder.hpp"
#include "cachequeue.hpp"
#include "mir2xmapdata.hpp"
#include "serverobject.hpp"
#include "batchluamodule.hpp"

class Player;
class NPChar;
class Monster;
class ServiceCore;
class ServerObject;

class ServerMap final: public ServerObject
{
    private:
        class ServerMapLuaModule: public ServerLuaModule
        {
            private:
                sol::coroutine m_coHandler;

            public:
                ServerMapLuaModule(ServerMap *);

            public:
                void resumeLoop()
                {
                    if(!m_coHandler){
                        throw fflerror("ServerMap lua coroutine is not callable");
                    }
                    checkResult(m_coHandler());
                }

            private:
                template<typename T> void checkResult(const T &result)
                {
                    if(result.valid()){
                        return;
                    }

                    const sol::error err = result;
                    std::stringstream errStream(err.what());

                    std::string errLine;
                    while(std::getline(errStream, errLine, '\n')){
                        addLog(1, to_u8cstr(errLine));
                    }
                }
        };

    private:
        // bind to servermap
        // only for server map internal usage
        class ServerPathFinder: public AStarPathFinder
        {
            private:
                const ServerMap *m_map;

            private:
                const int m_checkCO;

            public:
                ServerPathFinder(const ServerMap*, int, int);
               ~ServerPathFinder() = default;
        };

        // the server pathfinder will call canMove() etc
        // which I refuse to set as public since it's dynamically updated
        friend class ServerPathFinder;

    private:
        struct MapCell
        {
            bool Locked;
            std::vector<uint64_t> UIDList;

            uint32_t mapID;
            int      switchX;
            int      switchY;

            CacheQueue<CommonItem, SYS_MAXDROPITEM> GroundItemQueue;

            MapCell()
                : Locked(false)
                , mapID(0)
                , switchX(-1)
                , switchY(-1)
                , GroundItemQueue()
            {}

            bool empty() const
            {
                return !Locked && UIDList.empty() && (mapID == 0) && GroundItemQueue.Empty();
            }
        };

    private:
        template<typename T> using Vec2D = std::vector<std::vector<T>>;

    private:
        const uint32_t     m_ID;
        const Mir2xMapData m_mir2xMapData;

    private:
        ServiceCore *m_serviceCore;

    private:
        Vec2D<MapCell> m_cellVec2D;

    private:
        std::unique_ptr<ServerMapLuaModule> m_luaModulePtr;

    private:
        void operateAM(const MessagePack &);

    public:
        ServerMap(ServiceCore *, uint32_t);
       ~ServerMap() = default;

    public:
        uint32_t ID() const { return m_ID; }

    public:
        bool In(uint32_t nMapID, int nX, int nY) const
        {
            return (nMapID == m_ID) && ValidC(nX, nY);
        }

    public:
        bool groundValid(int, int) const;

    protected:
        bool canMove(bool, bool, int, int) const;

    protected:
        double OneStepCost(int, int, int, int, int, int) const;

    public:
        const Mir2xMapData &GetMir2xMapData() const
        {
            return m_mir2xMapData;
        }

    public:
        int W() const
        {
            return m_mir2xMapData.Valid() ? m_mir2xMapData.W() : 0;
        }

        int H() const
        {
            return m_mir2xMapData.Valid() ? m_mir2xMapData.H() : 0;
        }

    public:
        bool ValidC(int nX, int nY) const
        {
            return m_mir2xMapData.ValidC(nX, nY);
        }

        bool ValidP(int nX, int nY) const
        {
            return m_mir2xMapData.ValidP(nX, nY);
        }

    public:
        uint64_t activate();

    private:
        bool Load(const char *);

    private:
        void    addGridUID(uint64_t, int, int, bool);
        bool    hasGridUID(uint64_t, int, int) const;
        void removeGridUID(uint64_t, int, int);

    private:
        [[maybe_unused]] std::tuple<bool, int, int> GetValidGrid(bool, bool, int) const;
        [[maybe_unused]] std::tuple<bool, int, int> GetValidGrid(bool, bool, int, int, int) const;

    private:
        void notifyNewCO(uint64_t, int, int);

    private:
        Player  *addPlayer (uint32_t,      int, int, int, bool);
        NPChar  *addNPChar (uint16_t,      int, int, int, bool);
        Monster *addMonster(uint32_t, uint64_t, int, int, bool);

    private:
        int GetMonsterCount(uint32_t);
        std::vector<std::u8string> getMonsterList() const;

    private:
        auto &getCell(int nX, int nY)
        {
            if(!ValidC(nX, nY)){
                throw fflerror("invalid location: x = %d, y = %d", nX, nY);
            }
            return m_cellVec2D[nX][nY];
        }

        const auto &getCell(int nX, int nY) const
        {
            if(!ValidC(nX, nY)){
                throw fflerror("invalid location: x = %d, y = %d", nX, nY);
            }
            return m_cellVec2D[nX][nY];
        }

    private:
        auto &getUIDList(int nX, int nY)
        {
            return getCell(nX, nY).UIDList;
        }

        const auto &getUIDList(int nX, int nY) const
        {
            return getCell(nX, nY).UIDList;
        }

    private:
        auto &GetGroundItemList(int nX, int nY)
        {
            return getCell(nX, nY).GroundItemQueue;
        }

        const auto &GetGroundItemList(int nX, int nY) const
        {
            return getCell(nX, nY).GroundItemQueue;
        }

    private:
        int FindGroundItem(const CommonItem &, int, int);
        int GroundItemCount(const CommonItem &, int, int);

        bool AddGroundItem(const CommonItem &, int, int);
        void RemoveGroundItem(const CommonItem &, int, int);

        void ClearGroundItem(int, int);

    private:
        int CheckPathGrid(int, int) const;

    private:
        template<std::predicate<uint64_t> F> bool doUIDList(int x, int y, const F &func)
        {
            if(!ValidC(x, y)){
                return false;
            }

            for(const auto uid: getUIDList(x, y)){
                if(func(uid)){
                    return true;
                }
            }
            return false;
        }

    private:
        template<std::predicate<int, int> F> bool doCircle(int cx0, int cy0, int r, const F &f)
        {
            int doW = 2 * r - 1;
            int doH = 2 * r - 1;

            int x0 = cx0 - r + 1;
            int y0 = cy0 - r + 1;

            if((doW > 0) && (doH > 0) && mathf::rectangleOverlapRegion(0, 0, W(), H(), &x0, &y0, &doW, &doH)){
                for(int x = x0; x < x0 + doW; ++x){
                    for(int y = y0; y < y0 + doH; ++y){
                        if(true || ValidC(x, y)){
                            if(mathf::LDistance2(x, y, cx0, cy0) <= (r - 1) * (r - 1)){
                                if(f(x, y)){
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
            return false;
        }

        template<std::predicate<int, int> F> bool doSquare(int x0, int y0, int doW, int doH, const F &f)
        {
            if((doW > 0) && (doH > 0) && mathf::rectangleOverlapRegion(0, 0, W(), H(), &x0, &y0, &doW, &doH)){
                for(int x = x0; x < x0 + doW; ++x){
                    for(int y = y0; y < y0 + doH; ++y){
                        if(true || ValidC(x, y)){
                            if(f(x, y)){
                                return true;
                            }
                        }
                    }
                }
            }
            return false;
        }

        bool DoCenterCircle(int, int, int,      bool, const std::function<bool(int, int)> &);
        bool DoCenterSquare(int, int, int, int, bool, const std::function<bool(int, int)> &);

    private:
        void on_MPK_ACTION(const MessagePack &);
        void on_MPK_PICKUP(const MessagePack &);
        void on_MPK_OFFLINE(const MessagePack &);
        void on_MPK_TRYMOVE(const MessagePack &);
        void on_MPK_TRYLEAVE(const MessagePack &);
        void on_MPK_PATHFIND(const MessagePack &);
        void on_MPK_UPDATEHP(const MessagePack &);
        void on_MPK_METRONOME(const MessagePack &);
        void on_MPK_PULLCOINFO(const MessagePack &);
        void on_MPK_BADACTORPOD(const MessagePack &);
        void on_MPK_DEADFADEOUT(const MessagePack &);
        void on_MPK_NEWDROPITEM(const MessagePack &);
        void on_MPK_TRYMAPSWITCH(const MessagePack &);
        void on_MPK_QUERYCOCOUNT(const MessagePack &);
        void on_MPK_TRYSPACEMOVE(const MessagePack &);
        void on_MPK_ADDCHAROBJECT(const MessagePack &);
        void on_MPK_QUERYRECTUIDLIST(const MessagePack &);

    private:
        bool regLuaExport(ServerMapLuaModule *);
};
