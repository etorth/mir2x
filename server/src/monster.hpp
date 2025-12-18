#pragma once
#include <optional>
#include <functional>
#include <unordered_set>
#include "corof.hpp"
#include "fflerror.hpp"
#include "battleobject.hpp"
#include "dbcomid.hpp"
#include "monsterrecord.hpp"

class Monster: public BattleObject
{
    public:
        friend class ServerObject;
        friend class   CharObject;
        friend class BattleObject;

    protected:
        // a-star algorithm is so expensive
        // current logic is every step we do moveOneStep()

        // cache the a-star result helps
        // but it makes monster stop if path node is blocked
        // so I need to keep tracking time to refresh the cache

        class AStarCache
        {
            private:
                // we refresh (drop) the cache every 2 sec
                // since co's are moving and valid road won't keep valid
                // 2 sec because many monsters have WalkWait = 1sec
                constexpr static uint32_t m_refresh = 2000;

            private:
                uint32_t m_time = 0;
                uint32_t m_mapID = 0;
                std::vector<pathf::PathNode> m_path;

            public:
                AStarCache() = default;

            public:
                void cache(uint32_t, std::vector<pathf::PathNode>);
                std::optional<pathf::PathNode> retrieve(uint32_t, int, int, int, int);
        };

    protected:
        enum FPMethodType: int
        {
            FPMETHOD_NONE,
            FPMETHOD_ASTAR,
            FPMETHOD_GREEDY,
            FPMETHOD_COMBINE,
            FPMETHOD_NEIGHBOR,
        };

    protected:
        friend class CharObject;

    protected:
        uint64_t m_masterUID;

    protected:
        AStarCache m_astarCache;

    protected:
        std::optional<std::pair<uint64_t, uint64_t>> m_idleWaitToken {};

    public:
        Monster(uint32_t,           // monster id
                uint64_t,           // server map uid
                int,                // map x
                int,                // map y
                int,                // direction
                uint64_t);          // master uid

    public:
        ~Monster() = default;

    public:
        uint32_t monsterID() const
        {
            return uidf::getMonsterID(UID());
        }

    protected:
        // don't expose it to public
        // master may change by time or by magic
        uint64_t masterUID() const
        {
            return m_masterUID;
        }

    protected:
        void SearchViewRange();

    protected:
        virtual corof::awaitable<> runAICoro();

    protected:
        bool randomTurn();
        virtual corof::awaitable<bool> randomMove();
        virtual corof::awaitable<bool> moveForward();
        virtual corof::awaitable<bool> followMaster();

    protected:
        corof::awaitable<uint64_t> searchNearestTarget();

    protected:
        virtual corof::awaitable<bool> jumpUID       (uint64_t             );
        virtual corof::awaitable<bool> trackUID      (uint64_t, DCCastRange);
        virtual corof::awaitable<bool> attackUID     (uint64_t,         int);
        virtual corof::awaitable<bool> jumpAttackUID (uint64_t             );
        virtual corof::awaitable<bool> trackAttackUID(uint64_t             );

    protected:
        bool dcValid(int, bool);

    protected:
        bool struckDamage(uint64_t, const DamageNode &) override;

    protected:
        DamageNode getAttackDamage(int, int) const override;

    private:
        corof::awaitable<> on_AM_EXP             (const ActorMsgPack &);
        corof::awaitable<> on_AM_MISS            (const ActorMsgPack &);
        corof::awaitable<> on_AM_HEAL            (const ActorMsgPack &);
        corof::awaitable<> on_AM_ATTACK          (const ActorMsgPack &);
        corof::awaitable<> on_AM_ADDBUFF         (const ActorMsgPack &);
        corof::awaitable<> on_AM_REMOVEBUFF      (const ActorMsgPack &);
        corof::awaitable<> on_AM_ACTION          (const ActorMsgPack &);
        corof::awaitable<> on_AM_OFFLINE         (const ActorMsgPack &);
        corof::awaitable<> on_AM_UPDATEHP        (const ActorMsgPack &);
        corof::awaitable<> on_AM_MAPSWITCHTRIGGER(const ActorMsgPack &);
        corof::awaitable<> on_AM_MASTERKILL      (const ActorMsgPack &);
        corof::awaitable<> on_AM_MASTERHITTED    (const ActorMsgPack &);
        corof::awaitable<> on_AM_NOTIFYDEAD      (const ActorMsgPack &);
        corof::awaitable<> on_AM_BADACTORPOD     (const ActorMsgPack &);
        corof::awaitable<> on_AM_CHECKMASTER     (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYMASTER     (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYHEALTH     (const ActorMsgPack &);
        corof::awaitable<> on_AM_DEADFADEOUT     (const ActorMsgPack &);
        corof::awaitable<> on_AM_NOTIFYNEWCO     (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYUIDBUFF    (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYCORECORD   (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYLOCATION   (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYNAMECOLOR  (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYFRIENDTYPE (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYFINALMASTER(const ActorMsgPack &);

    protected:
        corof::awaitable<> onActorMsg(const ActorMsgPack &) override;

    protected:
        void reportCO(uint64_t) override;

    protected:
        corof::awaitable<bool> moveOneStep(int, int);

    protected:
        virtual corof::awaitable<bool> validTarget(uint64_t);

    protected:
        virtual corof::awaitable<uint64_t> pickTarget();
        virtual corof::awaitable<uint64_t> pickHealTarget();

    protected:
        virtual int  getAttackMagic(uint64_t) const;

    protected:
        int FindPathMethod();

    protected:
        corof::awaitable<int> checkFriend(uint64_t) override;
        corof::awaitable<int> queryPlayerFriend(uint64_t, uint64_t);

    private:
        corof::awaitable<int> checkFriend_ctrlByPlayer (uint64_t);
        corof::awaitable<int> checkFriend_ctrlByMonster(uint64_t);

    protected:
        corof::awaitable<bool> moveOneStepAStar   (int, int);
        corof::awaitable<bool> moveOneStepGreedy  (int, int);
        corof::awaitable<bool> moveOneStepCombine (int, int);
        corof::awaitable<bool> moveOneStepNeighbor(int, int);

    public:
        corof::awaitable<> onActivate() override;

    protected:
        bool canMove(bool)   const override;
        bool canAttack(bool) const override;

    protected:
        void onDie() override;

    protected:
        virtual corof::awaitable<bool> needHeal(uint64_t);
        virtual corof::awaitable<bool> inDCCastRange(uint64_t, DCCastRange);

    public:
        const auto &getMR() const
        {
            return DBCOM_MONSTERRECORD(uidf::getMonsterID(UID()));
        }

    public:
        std::u8string_view monsterName() const
        {
            return getMR().name;
        }

    public:
        bool hasPlayerNeighbor() const;

    protected:
        virtual corof::awaitable<bool> asyncIdleWait(uint64_t);

    protected:
        virtual corof::awaitable<> onAMAttack      (const ActorMsgPack &);
        virtual corof::awaitable<> onAMMasterHitted(const ActorMsgPack &);

    protected:
        void dispatchOffenderExp();
        void addOffenderDamage(uint64_t, int);
};
