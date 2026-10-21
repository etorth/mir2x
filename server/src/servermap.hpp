#pragma once
#include <array>
#include <map>
#include <set>
#include <tuple>
#include <memory>
#include <vector>
#include <cstdint>
#include <concepts>
#include <unordered_map>
#include <unordered_set>

#include "logprof.hpp"
#include "pathf.hpp"
#include "mathf.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "serdesmsg.hpp"
#include "actormsg.hpp"
#include "enableaddco.hpp"
#include "mir2xmapdata.hpp"
#include "serverobject.hpp"
#include "lochashtable.hpp"
#include "parallel_hashmap/phmap.h"

class ServerGuard;
class Player;
class NPChar;
class Monster;
class ServiceCore;
class ServerObject;

class ServerMap final: public ServerObject
{
    protected:
        class LuaThreadRunner: public ServerObject::LuaThreadRunner
        {
            public:
                LuaThreadRunner(ServerMap *);

            public:
                ServerMap *getServerMap() const
                {
                    return static_cast<ServerMap *>(m_actorPod->getSO());
                }
        };

    private:
        class ServerPathFinder: public pathf::AStarPathFinder
        {
            private:
                const ServerMap *m_map;

            private:
                const int m_checkCO;

            public:
               /* ctor */  ServerPathFinder(const ServerMap*, int, int);
               /* dtor */ ~ServerPathFinder() = default;
        };

    private:
        class EnableServerMapAddCO: public EnableAddCO
        {
            public:
                explicit EnableServerMapAddCO(ActorPod *);
                corof::awaitable<> onMsgAddCO(const ActorMsgPack &) override;
        };

    private:
        friend class ServerPathFinder;
        friend class ServerMap::LuaThreadRunner;

    private:
        struct FireWallMagicNode
        {
            uint64_t uid = 0;
            uint64_t groundSeq = 0;

            int minDC = 0;
            int maxDC = 0;
            int mcHit = 0;

            uint64_t startTime      = 0;
            uint64_t lastAttackTime = 0;

            int duration = 0;
            int dps      = 0;
        };

        struct DroppedItemNode
        {
            SDItem item;
            uint64_t groundSeq = 0; // items dropped in one batch share one groundSeq
        };

        struct MapGrid
        {
            bool locked = false;
            std::vector<uint64_t> uidList;
            std::vector<uint64_t> observerList;
            std::vector<DroppedItemNode> itemList;
            std::vector<FireWallMagicNode> fireWallList;

            uint64_t mapUID  =  0;
            int      switchX = -1;
            int      switchY = -1;

            // gridTriggerIds of every grid trigger covering this grid
            //
            // each id belongs to one _RSVD_NAME_allocateGridTriggerId() call, and the rect
            // list of that id lives in m_gridTriggerList. when non-empty the automatic
            // switch is skipped and the map script decides instead, see addGridTrigger()
            // in servermap.lua
            std::vector<int> triggerList;

            bool empty() const
            {
                return !locked && uidList.empty() && observerList.empty() && itemList.empty() && fireWallList.empty() && (mapUID == 0) && triggerList.empty();
            }

            bool hasUID(uint64_t uid) const
            {
                return std::ranges::find(uidList, uid) != uidList.end();
            }
        };

    private:
        bool m_closing = false;

    private:
        std::unique_ptr<EnableAddCO> m_addCO;

    private:
        std::shared_ptr<Mir2xMapData> m_mapBin;

    private:
        std::unordered_map<std::string, uint64_t> m_npcList;

    private:
        uint64_t m_groundSeq = 1; // allocates groundSeq for each firewall and each ground item drop batch
        std::vector<MapGrid> m_gridList;

    private:
        // grid trigger registry, see ServerMap::allocateGridTriggerId()
        //
        // gridTriggerId -> the rect array of that trigger, each rect is {x, y, w, h}; the
        // grid -> gridTriggerId direction lives in each MapGrid::triggerList
        std::unordered_map<int, std::vector<std::array<int, 4>>> m_gridTriggerList;

        // next free gridTriggerId handed out by allocateGridTriggerId()
        int m_gridTriggerID = 0;

    private:
        const uint64_t m_mainScriptThreadKey = 1;
        /* */ uint64_t m_threadKey = m_mainScriptThreadKey + 1;

    private:
        std::unique_ptr<ServerMap::LuaThreadRunner> m_luaRunner;

    private:
        void beforeActivate() override;
        corof::awaitable<> onActorMsg(const ActorMsgPack &) override;

    private:
        // a player finished a move or jump onto (x, y), send it to the next map if that grid
        // leads anywhere, unless a trigger owns this grid and takes the decision over
        void dispatchGridSwitch(uint64_t, int, int);

    private:
        // allocate a fresh gridTriggerId for a rect array of grid trigger, each rect is
        // {x, y, w, h}; removeGridTriggerId() restores the grid state
        int  allocateGridTriggerId(const std::vector<std::array<int, 4>> &);
        void removeGridTriggerId(int);

    public:
        ServerMap(uint64_t);

    private:
        ~ServerMap() = default;

    public:
        const Mir2xMapData *mapBin() const
        {
            return m_mapBin.get();
        }

    public:
        uint32_t ID() const
        {
            return uidf::getMapID(UID());
        }

    public:
        bool in(uint64_t argMapUID, int argX, int argY) const
        {
            return (argMapUID == UID()) && mapBin()->validC(argX, argY);
        }

    public:
        // safe zone is runtime configurable, it does not belong to the map bin
        //
        // ServerMap owns it, but combat decisions are made by the actors themselves, so this
        // barely-changing info gets broadcasted to every actor on the map when it changes, and
        // an actor which just entered the map pulls it, see BattleObject::isSafeGrid()
        //
        // TODO no way to configure a safe zone yet, so no grid is safe
        bool isSafeGrid(int, int) const;

    protected:
        bool canMove(bool, bool, int, int) const;

    protected:
        std::optional<double> oneStepCost(int, int, int, int, int, int, int) const;

    public:
        corof::awaitable<> onActivate() override;

    private:
        corof::awaitable<> loadNPChar();

    private:
        bool    hasGridUID(uint64_t, int, int) const;

        void   moveGridUID(std::pair<uint64_t, int>, int, int, int, int);
        void    addGridUID(std::pair<uint64_t, int>, int, int, bool, bool = true);
        bool removeGridUID(std::pair<uint64_t, int>, int, int,       bool = true);

        void   updateGridObserverCircle(std::pair<uint64_t, int>, int, int, bool);
        void     moveGridObserverCircle(std::pair<uint64_t, int>, int, int, int, int);

        void   updateGridObserver      (uint64_t, int, int, bool);
        void   resizeGridObserverCircle(uint64_t, int, int, int, int);
        void notifyGridObserved        (uint64_t, int, int);

        void forwardObserverAction(const AMAction &, int, int);
        void forwardObserverAction(const AMAction &, int, int, int, int);
        void notifyCODisappear(uint64_t, int, int);

    private:
        [[maybe_unused]] std::optional<std::tuple<int, int>> getRCGLoc(bool, bool, int, int, int, int, int, int, int) const;

    private:
        [[maybe_unused]] std::optional<std::tuple<int, int>> getRCValidGrid(bool, bool, int) const;
        [[maybe_unused]] std::optional<std::tuple<int, int>> getRCValidGrid(bool, bool, int, int, int) const;
        [[maybe_unused]] std::optional<std::tuple<int, int>> getRCValidGrid(bool, bool, int, int, int, int, int) const;

    private:
        int getMonsterCount(uint32_t);

    private:
        auto & getGrid(this auto && self, int x, int y)
        {
            fflassert(self.mapBin()->validC(x, y));
            return self.m_gridList.at(x + y * self.mapBin()->w());
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
        const auto &getGridItemList(int x, int y) const
        {
            return getGrid(x, y).itemList;
        }

        auto &getGridItemList(int x, int y)
        {
            return getGrid(x, y).itemList;
        }

    private:
        auto getGridItemIDList(int x, int y) const
        {
            std::vector<uint32_t> itemIDList;
            const auto &itemList = getGridItemList(x, y);

            itemIDList.reserve(itemList.size());
            for(const auto &item: itemList){
                itemIDList.push_back(item.item.itemID);
            }
            return itemIDList;
        }

    private:
        void clearGridItemList(int x, int y, bool post = true)
        {
            getGridItemList(x, y).clear();
            if(post){
                postGridItemIDList(x, y);
            }
        }

    private:
        bool hasGridItemID(uint32_t, int, int) const;
        size_t getGridItemIDCount(uint32_t, int, int) const;

    private:
        void addGridItem(SDItem, uint64_t, int, int, bool = true);

    private:
        void    addGridItemID(uint32_t, int, int, bool = true);
        void removeGridItemID(uint32_t, int, int, bool = true);

    private:
        void expireGridItemLater(uint64_t, std::set<std::pair<int, int>>);

    private:
        SDGroundItemIDList getGroundItemIDList(int, int, size_t);

    private:
        void postGridItemIDList(int, int);
        void postGroundItemIDList(std::pair<uint64_t, int>, int, int);

    private:
        void runFireWall(int, int, uint64_t);
        void attackFireWall(int, int, const FireWallMagicNode &);

    private:
        SDGroundFireWallList getGroundFireWallList(int, int, size_t);

    private:
        void postGridFireWallList(int, int);
        void postGroundFireWallList(std::pair<uint64_t, int>, int, int);

    private:
        int checkPathGrid(int, int) const;

    private:
        template<std::predicate<uint64_t> F> bool doUIDList(int x, int y, const F &func)
        {
            if(!mapBin()->validC(x, y)){
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
        template<std::predicate<int, int> F> static bool doCircle(const Mir2xMapData *data, int cx0, int cy0, int r, const F &f)
        {
            int doW = 2 * r - 1;
            int doH = 2 * r - 1;

            int x0 = cx0 - r + 1;
            int y0 = cy0 - r + 1;

            if((doW > 0) && (doH > 0) && (!data || mathf::rectangleOverlapRegion<int>(0, 0, data->w(), data->h(), x0, y0, doW, doH))){
                for(int x = x0; x < x0 + doW; ++x){
                    for(int y = y0; y < y0 + doH; ++y){
                        if(!data || data->validC(x, y)){
                            if(mathf::LDistance2<int>(x, y, cx0, cy0) <= (r - 1) * (r - 1)){
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

        template<std::predicate<int, int> F> bool doCircle(this auto && self, int cx0, int cy0, int r, const F &f)
        {
            return ServerMap::doCircle(self.mapBin(), cx0, cy0, r, f);
        }


    private:
        // addDeleteList:
        // {dir, step, radius} -> pair{ first : vector_of_{dx, dy},
        //                              second: vector_of_{dx, dy}}
        //
        inline const static auto m_addDeleteGridList = []
        {
            logScopedProfiler("addDeleteGridListInit");
            const auto fnGetCircle = [](int cx, int cy, int r) -> std::set<std::pair<int, int>>
            {
                std::set<std::pair<int, int>> circle;
                ServerMap::doCircle(nullptr, cx, cy, r, [&circle](int x, int y)
                {
                    circle.emplace(x, y);
                    return false;
                });
                return circle;
            };

            constexpr int MAX_CACHED_R    = SYS_MAX_VIEWR;
            constexpr int MAX_CACHED_STEP = 3;

            std::map<std::tuple<int, int, int>,
                /**/ std::pair<std::vector<std::pair<int, int>>,
                /**/           std::vector<std::pair<int, int>>>> result;

            for(int r = 1; r <= MAX_CACHED_R; ++r){
                const auto oldCircle = fnGetCircle(0, 0, r);
                for(int dir = DIR_BEGIN; dir < DIR_END; ++dir){
                    for(int step = 1; step <= MAX_CACHED_STEP; ++step){
                        const auto [dx, dy] = pathf::getFrontGLoc(0, 0, dir, step);
                        const auto newCircle = fnGetCircle(dx, dy, r);

                        for(const auto &loc: newCircle){
                            if(oldCircle.find(loc) == oldCircle.end()){
                                result[{dir, step, r}].first.emplace_back(loc); // added grids
                            }
                        }

                        for(const auto &loc: oldCircle){
                            if(newCircle.find(loc) == newCircle.end()){
                                result[{dir, step, r}].second.emplace_back(loc); // removed grids
                            }
                        }
                    }
                }
            }
            return result;
        }();

    private:
        template<std::predicate<int, int> F> bool doSquare(this auto && self, int x0, int y0, int doW, int doH, const F &f)
        {
            if((doW > 0) && (doH > 0) && mathf::rectangleOverlapRegion<int>(0, 0, self.mapBin()->w(), self.mapBin()->h(), x0, y0, doW, doH)){
                for(int x = x0; x < x0 + doW; ++x){
                    for(int y = y0; y < y0 + doH; ++y){
                        if(self.mapBin()->validC(x, y)){
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
        corof::awaitable<> on_AM_ACTION              (const ActorMsgPack &);
        corof::awaitable<> on_AM_CLOSEMAP            (const ActorMsgPack &);
        corof::awaitable<> on_AM_PICKUP              (const ActorMsgPack &);
        corof::awaitable<> on_AM_OFFLINE             (const ActorMsgPack &);
        corof::awaitable<> on_AM_TRYJUMP             (const ActorMsgPack &);
        corof::awaitable<> on_AM_TRYMOVE             (const ActorMsgPack &);
        corof::awaitable<> on_AM_TRYLEAVE            (const ActorMsgPack &);
        corof::awaitable<> on_AM_PATHFIND            (const ActorMsgPack &);
        corof::awaitable<> on_AM_OBSERVERNETPACKAGE  (const ActorMsgPack &);
        corof::awaitable<> on_AM_BADACTORPOD         (const ActorMsgPack &);
        corof::awaitable<> on_AM_DEADFADEOUT         (const ActorMsgPack &);
        corof::awaitable<> on_AM_DROPITEMLIST        (const ActorMsgPack &);
        corof::awaitable<> on_AM_TRYMAPSWITCH        (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYCOCOUNT        (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYOBSERVED       (const ActorMsgPack &);
        corof::awaitable<> on_AM_VIEWRANGE           (const ActorMsgPack &);
        corof::awaitable<> on_AM_TRYSPACEMOVE        (const ActorMsgPack &);
        corof::awaitable<> on_AM_CASTFIREWALL        (const ActorMsgPack &);
        corof::awaitable<> on_AM_REMOTECALL          (const ActorMsgPack &);
        corof::awaitable<> on_AM_STRIKEFIXEDLOCDAMAGE(const ActorMsgPack &);
};
