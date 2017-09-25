/*
 * =====================================================================================
 *
 *       Filename: servermap.hpp
 *        Created: 09/03/2015 03:49:00
 *  Last Modified: 09/13/2017 12:24:44
 *
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
#include "uidrecord.hpp"
#include "metronome.hpp"
#include "commonitem.hpp"
#include "pathfinder.hpp"
#include "mir2xmapdata.hpp"
#include "activeobject.hpp"

class ServiceCore;
class ServerObject;
class ServerMap: public ActiveObject
{
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
        enum QueryType: int
        {
            QUERY_NA,
            QUERY_PENDING,
            QUERY_OK,
            QUERY_ERROR,
        };

    private:
        struct CellRecord
        {
            bool Lock;

            uint32_t UID;
            uint32_t MapID;
            int      SwitchX;
            int      SwitchY;

            // query service core for the map UID
            // for a map switch point record it's query state
            // can't use pServiceCore->GetMapUID() since map in service core could load / unload
            int Query;

            // on every grid there could be one item on ground
            std::array<CommonItem, SYS_MAXDROPITEM> GroundItemList;

            CellRecord()
                : Lock(false)
                , UID(0)
                , MapID(0)
                , SwitchX(-1)
                , SwitchY(-1)
                , Query(QUERY_NA)
                , GroundItemList()
            {}
        };

    private:
        template<typename T> using Vec2D = std::vector<std::vector<T>>;

    private:
        const uint32_t     m_ID;
        const Mir2xMapData m_Mir2xMapData;

    private:
        Metronome   *m_Metronome;
        ServiceCore *m_ServiceCore;

    private:
        Vec2D<CellRecord> m_CellRecordV2D;
        Vec2D<std::vector<uint32_t>> m_UIDRecordV2D;

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
        bool CanMove(bool, bool, bool, int, int, int, int);

    protected:
        double MoveCost(bool, bool, bool, int, int, int, int);

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

    private:
        bool Empty();
        bool RandomLocation(int *, int *);

    private:
        int  DropItemListCount(int, int);
        bool AddGroundItem(int, int, const CommonItem &);

    private:
        void On_MPK_ACTION(const MessagePack &, const Theron::Address &);
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
        void On_MPK_QUERYCORECORD(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYRECTUIDV(const MessagePack &, const Theron::Address &);
};
