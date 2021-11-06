#pragma once
#include <map>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>
#include "scopedalloc.hpp"
#include "bufflist.hpp"
#include "charobject.hpp"
#include "damagenode.hpp"
#include "actionnode.hpp"
#include "protocoldef.hpp"

class BattleObject: public CharObject
{
    public:
        friend class CharObject;

    protected:
        class BOPathFinder final: public AStarPathFinder
        {
            private:
                friend class BattleObject;

            private:
                const BattleObject *m_BO;

            private:
                const int m_checkCO;

            private:
                mutable std::map<uint32_t, int> m_cache;

            public:
                /* ctor */  BOPathFinder(const BattleObject *, int);
                /* dtor */ ~BOPathFinder() = default;

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
            uint64_t uid = 0;
            uint64_t damage = 0;
            uint64_t activeTime = 0;
        };

    protected:
        const ServerMap *GetServerMap() const
        {
            return m_map;
        }

    protected:
        SDHealth m_sdHealth;

    protected:
        BuffList m_buffList;

    protected:
        bool m_moveLock;
        bool m_attackLock;

    protected:
        int m_lastAction = ACTION_NONE;
        std::array<uint32_t, ACTION_END> m_lastActionTime;

    protected:
        std::vector<Offender> m_offenderList;

    public:
        BattleObject(
                const ServerMap *,  // server map
                uint64_t,           // uid
                int,                // map x
                int,                // map y
                int);               // direction

    public:
        ~BattleObject() = default;

    public:
        virtual bool update() = 0;

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
        void dispatchHealth(uint64_t);
        void dispatchAttackDamage(uint64_t, int);

    protected:
        virtual std::optional<std::tuple<int, int, int>> oneStepReach(int, int) const;

    protected:
        virtual int Speed(int) const;

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
        virtual bool canAct()    const;
        virtual bool canMove()   const;
        virtual bool canAttack() const;

    protected:
        void setLastAction(int);

    protected:
        virtual bool dcValid(int, bool) = 0;

    protected:
        virtual DamageNode getAttackDamage(int) const = 0;

    protected:
        virtual bool struckDamage(const DamageNode &) = 0;

    protected:
        void addMonster(uint32_t, int, int, bool);

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
        int attackSpeed() const
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
        int CheckPathGrid(int, int) const;
        double OneStepCost(const BattleObject::BOPathFinder *, int, int, int, int, int) const;

    protected:
        virtual void checkFriend(uint64_t, std::function<void(int)>) = 0;

    protected:
        void queryHealth(uint64_t, std::function<void(uint64_t, SDHealth)>);
        void queryFinalMaster(uint64_t, std::function<void(uint64_t)>);

    protected:
        bool isOffender(uint64_t);

    protected:
        template<typename... Args> void dispatchInViewCONetPackage(uint8_t type, Args && ... args)
        {
            for(const auto &[uid, coLoc]: m_inViewCOList){
                if(uidf::getUIDType(coLoc.uid) == UID_PLY){
                    forwardNetPackage(coLoc.uid, type, std::forward<Args>(args)...);
                }
            }
        }

    protected:
        void addBuff(uint32_t);
        void dispatchBuffIDList();

    protected:
        virtual void updateBuffList();

    protected:
        virtual bool updateHealth(
                std::optional<int> = {},    // hp
                std::optional<int> = {},    // mp
                std::optional<int> = {},    // maxHP
                std::optional<int> = {});   // maxMP
};
