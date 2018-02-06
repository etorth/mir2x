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

#include <vector>
#include <cstdint>

#include "sysconst.hpp"
#include "querytype.hpp"
#include "uidrecord.hpp"
#include "metronome.hpp"
#include "commonitem.hpp"
#include "pathfinder.hpp"
#include "mir2xmapdata.hpp"
#include "activeobject.hpp"
#include "batchluamodule.hpp"

class Player;
class Monster;
class ServiceCore;
class ServerObject;

class ServerMap final: public ActiveObject
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
            public:
                ServerPathFinder(ServerMap*, int, bool);
               ~ServerPathFinder() = default;
        };

        // the server pathfinder will call CanMove() etc
        // which I refuse to set as public since it's dynamically updated
        friend class ServerPathFinder;

    private:
        struct CellRecord
        {
            bool Lock;
            std::vector<uint32_t> UIDList;

            uint32_t UID;
            uint32_t MapID;
            int      SwitchX;
            int      SwitchY;

            // query service core for the map UID
            // for a map switch point record it's query state
            // can't use pServiceCore->GetMapUID() since map in service core could load / unload
            int Query;

            // on every grid there could be one item on ground
            CacheQueue<CommonItem, SYS_MAXDROPITEM> GroundItemQueue;

            CellRecord()
                : Lock(false)
                , UIDList()
                , UID(0)
                , MapID(0)
                , SwitchX(-1)
                , SwitchY(-1)
                , Query(QUERY_NONE)
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
        Vec2D<CellRecord> m_CellRecordV2D;

    private:
        ServerMapLuaModule *m_LuaModule;

    private:
        void OperateAM(const MessagePack &, const Theron::Address &);

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
        bool CanMove(bool, bool, int, int);
        bool CanMove(bool, bool, int, int, int, int);

    protected:
        double MoveCost(bool, bool, int, int, int, int);

    public:
        int W() const { return m_Mir2xMapData.Valid() ? m_Mir2xMapData.W() : 0; }
        int H() const { return m_Mir2xMapData.Valid() ? m_Mir2xMapData.H() : 0; }

        bool ValidC(int nX, int nY) const { return m_Mir2xMapData.ValidC(nX, nY); }
        bool ValidP(int nX, int nY) const { return m_Mir2xMapData.ValidP(nX, nY); }

    public:
        Theron::Address Activate();

    private:
        bool Load(const char *);

    private:
        void AddGridUID(uint32_t, int, int);
        void RemoveGridUID(uint32_t, int, int);

    private:
        bool Empty();
        bool RandomLocation(int *, int *);

    private:
        bool GetValidGrid(int *, int *, bool);

    private:
        void NotifyNewCO(uint32_t, int, int);

    private:
        Player  *AddPlayer (uint32_t, int, int, int, bool);
        Monster *AddMonster(uint32_t, uint32_t, int, int, bool);

    private:
        int GetMonsterCount(uint32_t);

    private:
        auto &GetUIDList(int nX, int nY)
        {
            return m_CellRecordV2D[nX][nY].UIDList;
        }

        const auto &GetUIDList(int nX, int nY) const
        {
            return m_CellRecordV2D[nX][nY].UIDList;
        }

    private:
        auto &GetGroundItemList(int nX, int nY)
        {
            return m_CellRecordV2D[nX][nY].GroundItemQueue;
        }

        const auto &GetGroundItemList(int nX, int nY) const
        {
            return m_CellRecordV2D[nX][nY].GroundItemQueue;
        }

    private:
        int FindGroundItem(const CommonItem &, int, int);
        int GroundItemCount(const CommonItem &, int, int);

        bool AddGroundItem(const CommonItem &, int, int);
        void RemoveGroundItem(const CommonItem &, int, int);

        void ClearGroundItem(int, int);

    private:
        bool DoUIDList(int, int, const std::function<bool(const UIDRecord &)> &);

    private:
        bool DoCircle(int, int, int,      const std::function<bool(int, int)> &);
        bool DoSquare(int, int, int, int, const std::function<bool(int, int)> &);

        bool DoCenterCircle(int, int, int,      bool, const std::function<bool(int, int)> &);
        bool DoCenterSquare(int, int, int, int, bool, const std::function<bool(int, int)> &);

    private:
        void On_MPK_ACTION(const MessagePack &, const Theron::Address &);
        void On_MPK_PICKUP(const MessagePack &, const Theron::Address &);
        void On_MPK_OFFLINE(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYMOVE(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYLEAVE(const MessagePack &, const Theron::Address &);
        void On_MPK_PATHFIND(const MessagePack &, const Theron::Address &);
        void On_MPK_UPDATEHP(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_PULLCOINFO(const MessagePack &, const Theron::Address &);
        void On_MPK_BADACTORPOD(const MessagePack &, const Theron::Address &);
        void On_MPK_DEADFADEOUT(const MessagePack &, const Theron::Address &);
        void On_MPK_NEWDROPITEM(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYMAPSWITCH(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYCOCOUNT(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYSPACEMOVE(const MessagePack &, const Theron::Address &);
        void On_MPK_ADDCHAROBJECT(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYRECTUIDLIST(const MessagePack &, const Theron::Address &);

    private:
        bool RegisterLuaExport(ServerMapLuaModule *);
};
