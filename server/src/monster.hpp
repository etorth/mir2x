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
        corof::eval_poller<> m_updateCoro;

    public:
        Monster(uint32_t,           // monster id
                const ServerMap *,  // server map
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
        bool update() override;

    protected:
        virtual corof::eval_poller<> updateCoroFunc();

    protected:
        bool randomMove();
        bool randomTurn();
        void followMaster(std::function<void()>, std::function<void()>);

    protected:
        void searchNearestTarget(std::function<void(uint64_t)>);
        void searchNearestTargetHelper(std::unordered_set<uint64_t>, std::function<void(uint64_t)>);

    protected:
        virtual void jumpUID       (uint64_t,              std::function<void()>, std::function<void()>);
        virtual void trackUID      (uint64_t, DCCastRange, std::function<void()>, std::function<void()>);
        virtual void attackUID     (uint64_t,         int, std::function<void()>, std::function<void()>);
        virtual void jumpAttackUID (uint64_t,              std::function<void()>, std::function<void()>);
        virtual void trackAttackUID(uint64_t,              std::function<void()>, std::function<void()>);

    protected:
        bool dcValid(int, bool);

    protected:
        bool struckDamage(uint64_t, const DamageNode &) override;

    protected:
        DamageNode getAttackDamage(int, int) const override;

    private:
        void on_AM_EXP             (const ActorMsgPack &);
        void on_AM_MISS            (const ActorMsgPack &);
        void on_AM_HEAL            (const ActorMsgPack &);
        void on_AM_ATTACK          (const ActorMsgPack &);
        void on_AM_ADDBUFF         (const ActorMsgPack &);
        void on_AM_REMOVEBUFF      (const ActorMsgPack &);
        void on_AM_ACTION          (const ActorMsgPack &);
        void on_AM_OFFLINE         (const ActorMsgPack &);
        void on_AM_UPDATEHP        (const ActorMsgPack &);
        void on_AM_METRONOME       (const ActorMsgPack &);
        void on_AM_MAPSWITCHTRIGGER(const ActorMsgPack &);
        void on_AM_MASTERKILL      (const ActorMsgPack &);
        void on_AM_MASTERHITTED    (const ActorMsgPack &);
        void on_AM_NOTIFYDEAD      (const ActorMsgPack &);
        void on_AM_BADACTORPOD     (const ActorMsgPack &);
        void on_AM_CHECKMASTER     (const ActorMsgPack &);
        void on_AM_QUERYMASTER     (const ActorMsgPack &);
        void on_AM_QUERYHEALTH     (const ActorMsgPack &);
        void on_AM_DEADFADEOUT     (const ActorMsgPack &);
        void on_AM_NOTIFYNEWCO     (const ActorMsgPack &);
        void on_AM_QUERYUIDBUFF    (const ActorMsgPack &);
        void on_AM_QUERYCORECORD   (const ActorMsgPack &);
        void on_AM_QUERYLOCATION   (const ActorMsgPack &);
        void on_AM_QUERYNAMECOLOR  (const ActorMsgPack &);
        void on_AM_QUERYFRIENDTYPE (const ActorMsgPack &);
        void on_AM_QUERYFINALMASTER(const ActorMsgPack &);

    protected:
        void operateAM(const ActorMsgPack &);

    protected:
        void reportCO(uint64_t) override;

    protected:
        bool moveOneStep(int, int, std::function<void()>, std::function<void()>);

    protected:
        virtual void pickTarget(std::function<void(uint64_t)>);
        virtual int  getAttackMagic(uint64_t) const;

    protected:
        int FindPathMethod();

    protected:
        void checkFriend(uint64_t, std::function<void(int)>) override;
        void queryPlayerFriend(uint64_t, uint64_t, std::function<void(int)>);

    private:
        void checkFriend_ctrlByPlayer (uint64_t, std::function<void(int)>);
        void checkFriend_ctrlByMonster(uint64_t, std::function<void(int)>);

    protected:
        void queryMaster(uint64_t, std::function<void(uint64_t)>);

    protected:
        bool moveOneStepAStar   (int, int, std::function<void()>, std::function<void()>);
        bool moveOneStepGreedy  (int, int, std::function<void()>, std::function<void()>);
        bool moveOneStepCombine (int, int, std::function<void()>, std::function<void()>);
        bool moveOneStepNeighbor(int, int, std::function<void()>, std::function<void()>);

    public:
        void onActivate() override
        {
            CharObject::onActivate();
            if(!masterUID()){
                return;
            }

            m_actorPod->forward(masterUID(), {AM_CHECKMASTER}, [this](const ActorMsgPack &rstRMPK)
            {
                if(rstRMPK.type() != AM_CHECKMASTEROK){
                    goDie();
                }
            });
        }

    protected:
        bool canMove()   const override;
        bool canAttack() const override;

    protected:
        virtual bool goDie();
        virtual bool goGhost();

    protected:
        virtual corof::eval_poller<bool>     coro_randomMove();
        virtual corof::eval_poller<bool>     coro_moveForward();
        virtual corof::eval_poller<bool>     coro_followMaster();
        virtual corof::eval_poller<bool>     coro_needHeal(uint64_t);
        virtual corof::eval_poller<uint64_t> coro_pickTarget();
        virtual corof::eval_poller<uint64_t> coro_pickHealTarget();
        virtual corof::eval_poller<int>      coro_checkFriend(uint64_t);
        virtual corof::eval_poller<bool>     coro_trackUID(uint64_t, DCCastRange);
        virtual corof::eval_poller<bool>     coro_attackUID(uint64_t, int);
        virtual corof::eval_poller<bool>     coro_jumpGLoc(int, int, int);
        virtual corof::eval_poller<bool>     coro_jumpUID(uint64_t);
        virtual corof::eval_poller<bool>     coro_jumpAttackUID(uint64_t);
        virtual corof::eval_poller<bool>     coro_trackAttackUID(uint64_t);
        virtual corof::eval_poller<bool>     coro_inDCCastRange(uint64_t, DCCastRange);
        virtual corof::eval_poller<bool>     coro_validTarget(uint64_t);
        virtual corof::eval_poller<std::optional<SDHealth>> coro_queryHealth(uint64_t);
        virtual corof::eval_poller<std::tuple<uint32_t, int, int>> coro_getCOGLoc(uint64_t);

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
        virtual void onAMAttack      (const ActorMsgPack &);
        virtual void onAMMasterHitted(const ActorMsgPack &);

    protected:
        void dispatchOffenderExp();
        void addOffenderDamage(uint64_t, int);
};
