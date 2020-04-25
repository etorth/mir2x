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
#include <vector>
#include <cstdint>

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
        class ServerMapLuaModule: public BatchLuaModule
        {
            public:
                ServerMapLuaModule();
               ~ServerMapLuaModule() = default;
        };

    private:
        // bind to servermap
        // only for server map internal usage
        class ServerPathFinder: public AStarPathFinder
        {
            private:
                const ServerMap *m_Map;

            private:
                const int m_CheckCO;

            public:
                ServerPathFinder(const ServerMap*, int, int);
               ~ServerPathFinder() = default;
        };

        // the server pathfinder will call CanMove() etc
        // which I refuse to set as public since it's dynamically updated
        friend class ServerPathFinder;

    private:
        struct MapCell
        {
            bool Locked;
            std::vector<uint64_t> UIDList;

            uint32_t MapID;
            int      SwitchX;
            int      SwitchY;

            CacheQueue<CommonItem, SYS_MAXDROPITEM> GroundItemQueue;

            MapCell()
                : Locked(false)
                , UIDList()
                , MapID(0)
                , SwitchX(-1)
                , SwitchY(-1)
                , GroundItemQueue()
            {}
        };

    private:
        template<typename T> using Vec2D = std::vector<std::vector<T>>;

    private:
        const uint32_t     m_ID;
        const Mir2xMapData m_Mir2xMapData;

    private:
        ServiceCore *m_ServiceCore;

    private:
        Vec2D<MapCell> m_CellVec2D;

    private:
        ServerMapLuaModule *m_LuaModule;

    private:
        void OperateAM(const MessagePack &);

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
        bool GroundValid(int, int) const;

    protected:
        bool CanMove(bool, bool, int, int) const;

    protected:
        double OneStepCost(int, int, int, int, int, int) const;

    public:
        const Mir2xMapData &GetMir2xMapData() const
        {
            return m_Mir2xMapData;
        }

    public:
        int W() const
        {
            return m_Mir2xMapData.Valid() ? m_Mir2xMapData.W() : 0;
        }

        int H() const
        {
            return m_Mir2xMapData.Valid() ? m_Mir2xMapData.H() : 0;
        }

    public:
        bool ValidC(int nX, int nY) const
        {
            return m_Mir2xMapData.ValidC(nX, nY);
        }

        bool ValidP(int nX, int nY) const
        {
            return m_Mir2xMapData.ValidP(nX, nY);
        }

    public:
        uint64_t Activate();

    private:
        bool Load(const char *);

    private:
        void    AddGridUID(uint64_t, int, int, bool);
        void RemoveGridUID(uint64_t, int, int);

    private:
        [[maybe_unused]] std::tuple<bool, int, int> GetValidGrid(bool, bool, int) const;
        [[maybe_unused]] std::tuple<bool, int, int> GetValidGrid(bool, bool, int, int, int) const;

    private:
        void notifyNewCO(uint64_t, int, int);

    private:
        Player  *AddPlayer (uint32_t,      int, int, int, bool);
        NPChar  *addNPChar (uint16_t,      int, int, int, bool);
        Monster *AddMonster(uint32_t, uint64_t, int, int, bool);

    private:
        int GetMonsterCount(uint32_t);

    private:
        auto &GetCell(int nX, int nY)
        {
            return m_CellVec2D[nX][nY];
        }

        const auto &GetCell(int nX, int nY) const
        {
            return m_CellVec2D[nX][nY];
        }

    private:
        auto &GetUIDListRef(int nX, int nY)
        {
            return m_CellVec2D[nX][nY].UIDList;
        }

        const auto &GetUIDListRef(int nX, int nY) const
        {
            return m_CellVec2D[nX][nY].UIDList;
        }

    private:
        auto &GetGroundItemList(int nX, int nY)
        {
            return m_CellVec2D[nX][nY].GroundItemQueue;
        }

        const auto &GetGroundItemList(int nX, int nY) const
        {
            return m_CellVec2D[nX][nY].GroundItemQueue;
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
        bool DoUIDList(int, int, const std::function<bool(uint64_t)> &);

    private:
        bool DoCircle(int, int, int,      const std::function<bool(int, int)> &);
        bool DoSquare(int, int, int, int, const std::function<bool(int, int)> &);

        bool DoCenterCircle(int, int, int,      bool, const std::function<bool(int, int)> &);
        bool DoCenterSquare(int, int, int, int, bool, const std::function<bool(int, int)> &);

    private:
        void On_MPK_ACTION(const MessagePack &);
        void On_MPK_PICKUP(const MessagePack &);
        void On_MPK_OFFLINE(const MessagePack &);
        void On_MPK_TRYMOVE(const MessagePack &);
        void On_MPK_TRYLEAVE(const MessagePack &);
        void On_MPK_PATHFIND(const MessagePack &);
        void On_MPK_UPDATEHP(const MessagePack &);
        void On_MPK_METRONOME(const MessagePack &);
        void On_MPK_PULLCOINFO(const MessagePack &);
        void On_MPK_BADACTORPOD(const MessagePack &);
        void On_MPK_DEADFADEOUT(const MessagePack &);
        void On_MPK_NEWDROPITEM(const MessagePack &);
        void On_MPK_TRYMAPSWITCH(const MessagePack &);
        void On_MPK_QUERYCOCOUNT(const MessagePack &);
        void On_MPK_TRYSPACEMOVE(const MessagePack &);
        void On_MPK_ADDCHAROBJECT(const MessagePack &);
        void On_MPK_QUERYRECTUIDLIST(const MessagePack &);

    private:
        bool RegisterLuaExport(ServerMapLuaModule *);
};
