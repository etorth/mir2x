#pragma once
#include <tuple>
#include <memory>
#include <vector>
#include <cstdint>
#include <concepts>
#include <unordered_set>

#include "pathf.hpp"
#include "mathf.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "serdesmsg.hpp"
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
        friend class ServerPathFinder;

    private:
        struct FireWallMagicNode
        {
            uint64_t uid = 0;

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
            uint64_t dropTime = 0;
        };

        struct MapGrid
        {
            bool locked = false;
            std::vector<uint64_t> uidList;
            std::vector<DroppedItemNode> itemList;
            std::vector<FireWallMagicNode> fireWallList;

            uint32_t mapID   =  0;
            int      switchX = -1;
            int      switchY = -1;

            bool empty() const
            {
                return !locked && uidList.empty() && itemList.empty() && fireWallList.empty() && (mapID == 0);
            }

            bool hasUID(uint64_t uid) const
            {
                return std::ranges::find(uidList, uid) != uidList.end();
            }
        };

    private:
        const uint32_t     m_ID;
        const Mir2xMapData m_mir2xMapData;

    private:
        std::unordered_map<uint64_t, const NPChar *> m_npcList;

    private:
        std::vector<MapGrid> m_gridList;
        phmap::flat_hash_set<std::tuple<int, int>, LocHashHelper> m_fireWallLocList;
        phmap::flat_hash_set<std::tuple<int, int>, LocHashHelper> m_gridItemLocList;

    private:
        const uint64_t m_mainScriptThreadKey = 1;
        /* */ uint64_t m_threadKey = m_mainScriptThreadKey + 1;

    private:
        std::unique_ptr<ServerMap::LuaThreadRunner> m_luaRunner;

    private:
        void operateAM(const ActorMsgPack &);

    public:
        ServerMap(uint32_t);

    private:
        ~ServerMap() = default;

    public:
        uint32_t ID() const { return m_ID; }

    public:
        bool in(uint32_t nMapID, int nX, int nY) const
        {
            return (nMapID == m_ID) && validC(nX, nY);
        }

    public:
        bool groundValid(int, int) const;

    protected:
        bool canMove(bool, bool, int, int) const;

    protected:
        std::optional<double> oneStepCost(int, int, int, int, int, int, int) const;

    public:
        const Mir2xMapData &getMapData() const
        {
            return m_mir2xMapData;
        }

    public:
        int W() const
        {
            return m_mir2xMapData.w();
        }

        int H() const
        {
            return m_mir2xMapData.h();
        }

    public:
        bool validC(int nX, int nY) const
        {
            return m_mir2xMapData.validC(nX, nY);
        }

        bool validP(int nX, int nY) const
        {
            return m_mir2xMapData.validP(nX, nY);
        }

    public:
        void onActivate() override;

    private:
        void loadNPChar();

    private:
        void    addGridUID(uint64_t, int, int, bool);
        bool    hasGridUID(uint64_t, int, int) const;
        bool removeGridUID(uint64_t, int, int);

    private:
        [[maybe_unused]] std::optional<std::tuple<int, int>> getRCGLoc(bool, bool, int, int, int, int, int, int, int) const;

    private:
        [[maybe_unused]] std::optional<std::tuple<int, int>> getRCValidGrid(bool, bool, int) const;
        [[maybe_unused]] std::optional<std::tuple<int, int>> getRCValidGrid(bool, bool, int, int, int) const;
        [[maybe_unused]] std::optional<std::tuple<int, int>> getRCValidGrid(bool, bool, int, int, int, int, int) const;

    private:
        void notifyNewCO(uint64_t, int, int);

    private:
        Player *addPlayer(const SDInitPlayer &);
        NPChar *addNPChar(const SDInitNPChar &);

    private:
        Monster *addMonster(uint32_t, uint64_t, int, int, bool);

    private:
        ServerGuard *addGuard(uint32_t, int, int, int);

    private:
        int getMonsterCount(uint32_t);

    private:
        const auto &getGrid(int x, int y) const
        {
            fflassert(validC(x, y));
            return m_gridList.at(x + y * W());
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
        void addGridItem(SDItem, int, int, bool = true);

    private:
        void    addGridItemID(uint32_t, int, int, bool = true);
        void removeGridItemID(uint32_t, int, int, bool = true);

    private:
        SDGroundItemIDList getGroundItemIDList(int, int, size_t); // may remove expired item

    private:
        void postGridItemIDList(int, int);
        void postGroundItemIDList(uint64_t, int, int);

    private:
        SDGroundFireWallList getGroundFireWallList(int, int, size_t); // may remove expired firewall

    private:
        void postGridFireWallList(int, int);
        void postGroundFireWallList(uint64_t, int, int);

    private:
        int checkPathGrid(int, int) const;

    private:
        void updateMapGrid();
        void updateMapGridFireWall();
        void updateMapGridGroundItem();

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

            if((doW > 0) && (doH > 0) && mathf::rectangleOverlapRegion(0, 0, W(), H(), x0, y0, doW, doH)){
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
            if((doW > 0) && (doH > 0) && mathf::rectangleOverlapRegion(0, 0, W(), H(), x0, y0, doW, doH)){
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
        void on_AM_ACTION              (const ActorMsgPack &);
        void on_AM_PICKUP              (const ActorMsgPack &);
        void on_AM_OFFLINE             (const ActorMsgPack &);
        void on_AM_TRYJUMP             (const ActorMsgPack &);
        void on_AM_TRYMOVE             (const ActorMsgPack &);
        void on_AM_TRYLEAVE            (const ActorMsgPack &);
        void on_AM_PATHFIND            (const ActorMsgPack &);
        void on_AM_UPDATEHP            (const ActorMsgPack &);
        void on_AM_METRONOME           (const ActorMsgPack &);
        void on_AM_BADACTORPOD         (const ActorMsgPack &);
        void on_AM_DEADFADEOUT         (const ActorMsgPack &);
        void on_AM_DROPITEM            (const ActorMsgPack &);
        void on_AM_TRYMAPSWITCH        (const ActorMsgPack &);
        void on_AM_QUERYCOCOUNT        (const ActorMsgPack &);
        void on_AM_TRYSPACEMOVE        (const ActorMsgPack &);
        void on_AM_CASTFIREWALL        (const ActorMsgPack &);
        void on_AM_ADDCO               (const ActorMsgPack &);
        void on_AM_REMOTECALL          (const ActorMsgPack &);
        void on_AM_STRIKEFIXEDLOCDAMAGE(const ActorMsgPack &);
};
