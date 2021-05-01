/*
 * =====================================================================================
 *
 *       Filename: charobject.hpp
 *        Created: 04/10/2016 12:05:22
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

#include <map>
#include <list>
#include <deque>
#include <vector>

#include "totype.hpp"
#include "fflerror.hpp"
#include "servermap.hpp"
#include "damagenode.hpp"
#include "actionnode.hpp"
#include "timedstate.hpp"
#include "cachequeue.hpp"
#include "scopedalloc.hpp"
#include "servicecore.hpp"
#include "protocoldef.hpp"
#include "serverobject.hpp"

enum _RangeType: uint8_t
{
    RANGE_VIEW,
    RANGE_MAP,
    RANGE_SERVER,

    RANGE_VISIBLE,
    RANGE_ATTACK,
    RANGE_TRACETARGET,
};

// cache entry for charobject location
// should be visible for CharObject and its derived classes
struct COLocation
{
    uint64_t UID;
    uint32_t mapID;
    uint32_t RecordTime;

    int X;
    int Y;
    int Direction;

    COLocation(
            uint64_t nUID        = 0,
            uint32_t nMapID      = 0,
            uint32_t nRecordTime = 0,

            int nX = -1,
            int nY = -1,
            int nDirection = DIR_NONE)
        : UID(nUID)
        , mapID(nMapID)
        , RecordTime(nRecordTime)
        , X(nX)
        , Y(nY)
        , Direction(nDirection)
    {}
};

class CharObject: public ServerObject
{
    protected:
        class COPathFinder final: public AStarPathFinder
        {
            private:
                friend class CharObject;

            private:
                const CharObject *m_CO;

            private:
                const int m_checkCO;

            private:
                mutable std::map<uint32_t, int> m_cache;

            public:
                COPathFinder(const CharObject *, int);
               ~COPathFinder() = default;

            private:
               int GetGrid(int, int) const;
        };

    protected:
        enum SpeedType: int
        {
            SPEED_NONE = 0,
            SPEED_MOVE,
            SPEED_ATTACK,
        };

    protected:
        struct Offender
        {
            uint64_t UID = 0;
            uint32_t Damage = 0;
            uint32_t ActiveTime = 0;

            Offender(uint64_t nUID = 0, uint32_t nDamage = 0, uint32_t nActiveTime = 0)
                : UID(nUID)
                , Damage(nDamage)
                , ActiveTime(nActiveTime)
            {}
        };

        struct Target
        {
            uint64_t UID = 0;
            hres_timer activeTimer;
        };

    protected:
        const ServiceCore *m_serviceCore;
        const ServerMap   *m_map;

    protected:
        const ServerMap *GetServerMap() const
        {
            return m_map;
        }

    protected:
        // list of all COs *this* CO can see
        // 1. used for path finding
        // 2. directly report to these COs for action non-moving
        // 3. need to report to map if moving
        // 4. part of these COs are neighbors if close enough
        // 5. don't remove COs in this list if expired, otherwise action in (2) may miss
        std::vector<COLocation> m_inViewCOList;

    protected:
        int m_X;
        int m_Y;
        int m_direction;

    protected:
        int m_HP;
        int m_HPMax;
        int m_MP;
        int m_MPMax;

    protected:
        bool     m_moveLock;
        bool     m_attackLock;
        uint32_t m_lastMoveTime;
        uint32_t m_lastAttackTime;

    protected:
        int      m_lastAction;
        uint32_t m_lastActionTime;

    protected:
        TimedState<bool> m_dead;

    protected:
        Target m_target;

    protected:
        std::vector<Offender> m_offenderList;

    public:
        CharObject(
                const ServiceCore *,    // service core
                const ServerMap   *,    // server map
                uint64_t,               // uid
                int,                    // map x
                int,                    // map y
                int);                   // direction

    public:
        ~CharObject() = default;

    protected:
        int X() const { return m_X; }
        int Y() const { return m_Y; }

    protected:
        int HP()    const { return m_HP; }
        int MP()    const { return m_MP; }
        int HPMax() const { return m_HPMax; }
        int MPMax() const { return m_MPMax; }

    protected:
        int Direction() const
        {
            return m_direction;
        }

        uint32_t mapID() const
        {
            return m_map ? m_map->ID() : 0;
        }

        uint64_t mapUID() const
        {
            return m_map->UID();
        }

    public:
        virtual bool update() = 0;
        virtual bool InRange(int, int, int) = 0;

    public:
        bool NextLocation(int *, int *, int, int);
        bool NextLocation(int *pX, int *pY, int nDistance)
        {
            return NextLocation(pX, pY, Direction(), nDistance);
        }

    public:
        void onActivate() override
        {
            ServerObject::onActivate();
            dispatchAction(ActionSpawn
            {
                .x = X(),
                .y = Y(),
                .direction = Direction(),
            });
        }

    protected:
        virtual void reportCO(uint64_t) = 0;

    protected:
        void dispatchHealth();
        void dispatchAttack(uint64_t, int);

    protected:
        virtual void dispatchAction(          const ActionNode &);
        virtual void dispatchAction(uint64_t, const ActionNode &);

    protected:
        virtual int OneStepReach(int, int, int *, int *);

    protected:
        virtual int Speed(int) const;

    protected:
        virtual bool canMove();

    protected:
        void retrieveLocation(uint64_t, std::function<void(const COLocation &)>, std::function<void()> = []{});

    protected:
        bool requestJump(
                int,                                // x
                int,                                // y
                int,                                // direction
                std::function<void()> = nullptr,    // fnOnOK
                std::function<void()> = nullptr);   // fnOnError

    protected:
        bool requestMove(
                int,                                // x
                int,                                // y
                int,                                // speed
                bool,                               // allowHalfMove
                bool,                               // removeMonster: force monster on (x, y) go to somewhere else to make room
                std::function<void()> = nullptr,    // fnOnOK
                std::function<void()> = nullptr);   // fnOnError

    protected:
        bool requestSpaceMove(
                int,                                // x
                int,                                // y
                bool,                               // strictMove
                std::function<void()> = nullptr,    // fnOnOK
                std::function<void()> = nullptr);   // fnOnError

    protected:
        bool requestMapSwitch(
                uint32_t,                           // mapID
                int,                                // x
                int,                                // y
                bool,                               // strictMove
                std::function<void()> = nullptr,    // fnOnOK
                std::function<void()> = nullptr);   // fnOnError

    protected:
        void addOffenderDamage(uint64_t, int);
        void dispatchOffenderExp();

    protected:
        virtual bool CanAct();
        virtual bool canAttack();

    protected:
        virtual void SetLastAction(int);

    protected:
        virtual bool DCValid(int, bool) = 0;

    protected:
        virtual DamageNode GetAttackDamage(int) = 0;

    protected:
        virtual bool StruckDamage(const DamageNode &) = 0;

    protected:
        void addMonster(uint32_t, int, int, bool);

    protected:
        virtual bool goDie()   = 0;
        virtual bool goGhost() = 0;

    protected:
        virtual int MaxStep() const
        {
            return 1;
        }

        virtual int MoveSpeed()
        {
            return SYS_DEFSPEED;
        }

    protected:
        // estimate how many hops we need
        // this function checks map but can't check CO
        // if we found one-hop distance we need send move request to servermap
        // return:
        //          -1: invalid
        //           0: no move needed
        //           1: one-hop can reach
        //           2: more than one-hop can reach
        int estimateHop(int, int);

    protected:
        int AttackSpeed() const
        {
            return SYS_DEFSPEED;
        }

        int MagicSpeed() const
        {
            return SYS_DEFSPEED;
        }

        int Horse() const
        {
            return 0;
        }

    protected:
        std::array<PathFind::PathNode, 3>    GetChaseGrid(int, int, int) const;
        std::vector<PathFind::PathNode> GetValidChaseGrid(int, int, int) const;

    protected:
        void GetValidChaseGrid(int, int, int, scoped_alloc::svobuf_wrapper<PathFind::PathNode, 3> &) const;

    protected:
        int CheckPathGrid(int, int, uint32_t = 0) const;
        double OneStepCost(const CharObject::COPathFinder *, int, int, int, int, int) const;

    protected:
        bool InView(uint32_t, int, int) const;

    protected:
        void SortInViewCO();
        void RemoveInViewCO(uint64_t);
        void AddInViewCO(const COLocation &);
        void AddInViewCO(uint64_t, uint32_t, int, int, int);
        void foreachInViewCO(std::function<void(const COLocation &)>);

    protected:
        COLocation &GetInViewCORef(uint64_t);
        COLocation *getInViewCOPtr(uint64_t);

    protected:
        virtual void checkFriend(uint64_t, std::function<void(int)>) = 0;

    protected:
        void QueryFinalMaster(uint64_t, std::function<void(uint64_t)>);

    protected:
        bool isOffender(uint64_t);

    protected:
        virtual void on_AM_QUERYFRIENDTYPE(const ActorMsgPack &);

    protected:
        bool isPlayer()  const;
        bool isMonster() const;

    protected:
        void notifyDead(uint64_t);

    protected:
        ActionNode makeActionStand() const;
};
