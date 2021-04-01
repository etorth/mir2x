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
#include "serdesmsg.hpp"
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
                        addLogString(1, to_u8cstr(errLine));
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
        struct MapGrid
        {
            bool locked = false;
            std::vector<uint64_t> uidList;
            std::vector<uint32_t> itemIDList;

            uint32_t mapID   =  0;
            int      switchX = -1;
            int      switchY = -1;

            bool empty() const
            {
                return !locked && uidList.empty() && itemIDList.empty() && (mapID == 0);
            }
        };

    private:
        const uint32_t     m_ID;
        const Mir2xMapData m_mir2xMapData;

    private:
        ServiceCore *m_serviceCore;

    private:
        std::vector<std::vector<MapGrid>> m_gridList;

    private:
        std::unique_ptr<ServerMapLuaModule> m_luaModulePtr;

    private:
        void operateAM(const ActorMsgPack &);

    public:
        ServerMap(ServiceCore *, uint32_t);
       ~ServerMap() = default;

    public:
        uint32_t ID() const { return m_ID; }

    public:
        bool In(uint32_t nMapID, int nX, int nY) const
        {
            return (nMapID == m_ID) && validC(nX, nY);
        }

    public:
        bool groundValid(int, int) const;

    protected:
        bool canMove(bool, bool, int, int) const;

    protected:
        double OneStepCost(int, int, int, int, int, int) const;

    public:
        const Mir2xMapData &getMapData() const
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
        bool validC(int nX, int nY) const
        {
            return m_mir2xMapData.ValidC(nX, nY);
        }

        bool validP(int nX, int nY) const
        {
            return m_mir2xMapData.ValidP(nX, nY);
        }

    public:
        void onActivate() override
        {
            ServerObject::onActivate();
            m_luaModulePtr = std::make_unique<ServerMap::ServerMapLuaModule>(this);
            loadNPChar();
        }

    private:
        void loadNPChar();

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
        Player *addPlayer(const SDInitPlayer &);
        NPChar *addNPChar(const SDInitNPChar &);

    private:
        Monster *addMonster(uint32_t, uint64_t, int, int, bool);

    private:
        int getMonsterCount(uint32_t);

    private:
        const auto &getGrid(int x, int y) const
        {
            if(!validC(x, y)){
                throw fflerror("invalid location: x = %d, y = %d", x, y);
            }
            return m_gridList.at(x).at(y);
        }

        auto &getGrid(int x, int y)
        {
            return const_cast<MapGrid &>(static_cast<const ServerMap *>(this)->getGrid(x, y));
        }

    private:
        const auto &getUIDList(int x, int y) const
        {
            return getGrid(x, y).uidList;
        }

        auto &getUIDList(int x, int y)
        {
            return getGrid(x, y).uidList;
        }

    private:
        const auto &getGridItemIDList(int x, int y) const
        {
            return getGrid(x, y).itemIDList;
        }

        auto &getGridItemIDList(int x, int y)
        {
            return getGrid(x, y).itemIDList;
        }

    private:
        void clearGridItemIDList(int x, int y, bool post = true)
        {
            getGridItemIDList(x, y).clear();
            if(post){
                postGridItemIDList(x, y);
            }
        }

    private:
        bool hasGridItemID(uint32_t, int, int) const;
        size_t getGridItemIDCount(uint32_t, int, int) const;

    private:
        void    addGridItemID(uint32_t, int, int, bool = true);
        void removeGridItemID(uint32_t, int, int, bool = true);

    private:
        SDGroundItemIDList getGroundItemIDList(int, int, size_t) const;

    private:
        void postGridItemIDList(int, int);
        void postGroundItemIDList(uint64_t, int, int);

    private:
        int CheckPathGrid(int, int) const;

    private:
        template<std::predicate<uint64_t> F> bool doUIDList(int x, int y, const F &func)
        {
            if(!validC(x, y)){
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
                        if(validC(x, y)){
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

        template<std::predicate<int, int> F> bool doCircle(int cx0, int cy0, int r, const F &f) const
        {
            return const_cast<ServerMap *>(this)->doCircle(cx0, cy0, r, f);
        }

        template<std::predicate<int, int> F> bool doSquare(int x0, int y0, int doW, int doH, const F &f)
        {
            if((doW > 0) && (doH > 0) && mathf::rectangleOverlapRegion(0, 0, W(), H(), &x0, &y0, &doW, &doH)){
                for(int x = x0; x < x0 + doW; ++x){
                    for(int y = y0; y < y0 + doH; ++y){
                        if(validC(x, y)){
                            if(f(x, y)){
                                return true;
                            }
                        }
                    }
                }
            }
            return false;
        }

        template<std::predicate<int, int> F> bool doSquare(int x0, int y0, int doW, int doH, const F &f) const
        {
            return const_cast<ServerMap *>(this)->doSquare(x0, y0, doW, doH, f);
        }

        bool DoCenterCircle(int, int, int,      bool, const std::function<bool(int, int)> &);
        bool DoCenterSquare(int, int, int, int, bool, const std::function<bool(int, int)> &);

    private:
        void on_AM_ACTION(const ActorMsgPack &);
        void on_AM_PICKUP(const ActorMsgPack &);
        void on_AM_OFFLINE(const ActorMsgPack &);
        void on_AM_TRYMOVE(const ActorMsgPack &);
        void on_AM_TRYLEAVE(const ActorMsgPack &);
        void on_AM_PATHFIND(const ActorMsgPack &);
        void on_AM_UPDATEHP(const ActorMsgPack &);
        void on_AM_METRONOME(const ActorMsgPack &);
        void on_AM_PULLCOINFO(const ActorMsgPack &);
        void on_AM_BADACTORPOD(const ActorMsgPack &);
        void on_AM_DEADFADEOUT(const ActorMsgPack &);
        void on_AM_NEWDROPITEM(const ActorMsgPack &);
        void on_AM_TRYMAPSWITCH(const ActorMsgPack &);
        void on_AM_QUERYCOCOUNT(const ActorMsgPack &);
        void on_AM_TRYSPACEMOVE(const ActorMsgPack &);
        void on_AM_ADDCHAROBJECT(const ActorMsgPack &);

    private:
        bool regLuaExport(ServerMapLuaModule *);
};
