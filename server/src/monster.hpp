/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 04/10/2016 02:32:45
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
#include <optional>
#include <functional>
#include <unordered_set>
#include "corof.hpp"
#include "fflerror.hpp"
#include "charobject.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"
#include "monsterrecord.hpp"

class Monster: public CharObject
{
    protected:
        // a-star algorithm is so expensive
        // current logic is every step we do MoveOneStep()

        // cache the a-star result helps
        // but it makes monster stop if path node is blocked
        // so I need to keep tracking time to refresh the cache

        struct AStarCache
        {
            // we refresh (drop) the cache every 2 sec
            // since co's are moving and valid road won't keep valid
            // 2sec because many monsters have WalkWait = 1sec
            const static uint32_t Refresh = 2000;

            uint32_t Time;
            uint32_t mapID;
            std::vector<PathFind::PathNode> Path;

            AStarCache();
            bool Retrieve(int *, int *, int, int, int, int, uint32_t);
            void Cache(std::vector<PathFind::PathNode>, uint32_t);
        };

    protected:
        enum FPMethodType: int
        {
            FPMETHOD_NONE,
            FPMETHOD_ASTAR,
            FPMETHOD_DSTAR,
            FPMETHOD_GREEDY,
            FPMETHOD_COMBINE,
            FPMETHOD_NEIGHBOR,
        };

    protected:
        friend class CharObject;

    protected:
        uint64_t m_masterUID;

    protected:
        AStarCache m_AStarCache;

    protected:
        corof::eval_poller m_updateCoro;

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
        virtual corof::eval_poller updateCoroFunc();

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
        bool DCValid(int, bool);

    protected:
        bool struckDamage(const DamageNode &) override;

    protected:
        DamageNode getAttackDamage(int) const override;

    private:
        void on_AM_EXP             (const ActorMsgPack &);
        void on_AM_MISS            (const ActorMsgPack &);
        void on_AM_HEAL            (const ActorMsgPack &);
        void on_AM_ATTACK          (const ActorMsgPack &);
        void on_AM_ACTION          (const ActorMsgPack &);
        void on_AM_OFFLINE         (const ActorMsgPack &);
        void on_AM_UPDATEHP        (const ActorMsgPack &);
        void on_AM_METRONOME       (const ActorMsgPack &);
        void on_AM_MAPSWITCH       (const ActorMsgPack &);
        void on_AM_MASTERKILL      (const ActorMsgPack &);
        void on_AM_MASTERHITTED    (const ActorMsgPack &);
        void on_AM_NOTIFYDEAD      (const ActorMsgPack &);
        void on_AM_BADACTORPOD     (const ActorMsgPack &);
        void on_AM_CHECKMASTER     (const ActorMsgPack &);
        void on_AM_QUERYMASTER     (const ActorMsgPack &);
        void on_AM_QUERYHEALTH     (const ActorMsgPack &);
        void on_AM_DEADFADEOUT     (const ActorMsgPack &);
        void on_AM_NOTIFYNEWCO     (const ActorMsgPack &);
        void on_AM_QUERYCORECORD   (const ActorMsgPack &);
        void on_AM_QUERYLOCATION   (const ActorMsgPack &);
        void on_AM_QUERYNAMECOLOR  (const ActorMsgPack &);
        void on_AM_QUERYFRIENDTYPE (const ActorMsgPack &);
        void on_AM_QUERYFINALMASTER(const ActorMsgPack &);

    protected:
        void operateAM(const ActorMsgPack &);

    protected:
        void reportCO(uint64_t);

    protected:
        bool MoveOneStep(int, int, std::function<void()>, std::function<void()>);

    protected:
        virtual void pickTarget(std::function<void(uint64_t)>);
        virtual int  getAttackMagic(uint64_t) const;

    protected:
        int FindPathMethod();

    protected:
        void QueryFriendType(uint64_t, uint64_t, std::function<void(int)>);
        void checkFriend(uint64_t, std::function<void(int)>) override;

    private:
        void checkFriend_CtrlByPlayer (uint64_t, std::function<void(int)>);
        void checkFriend_CtrlByMonster(uint64_t, std::function<void(int)>);

    protected:
        void queryMaster(uint64_t, std::function<void(uint64_t)>);

    protected:
        bool MoveOneStepAStar   (int, int, std::function<void()>, std::function<void()>);
        bool MoveOneStepDStar   (int, int, std::function<void()>, std::function<void()>);
        bool MoveOneStepGreedy  (int, int, std::function<void()>, std::function<void()>);
        bool MoveOneStepCombine (int, int, std::function<void()>, std::function<void()>);
        bool MoveOneStepNeighbor(int, int, std::function<void()>, std::function<void()>);

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
        corof::eval_awaiter<bool>     coro_randomMove();
        corof::eval_awaiter<bool>     coro_moveForward();
        corof::eval_awaiter<bool>     coro_followMaster();
        corof::eval_awaiter<uint64_t> coro_pickTarget();
        corof::eval_awaiter<uint64_t> coro_pickHealTarget();
        corof::eval_awaiter<int>      coro_checkFriend(uint64_t);
        corof::eval_awaiter<bool>     coro_trackUID(uint64_t, DCCastRange);
        corof::eval_awaiter<bool>     coro_attackUID(uint64_t, int);
        corof::eval_awaiter<bool>     coro_jumpGLoc(int, int, int);
        corof::eval_awaiter<bool>     coro_jumpUID(uint64_t);
        corof::eval_awaiter<bool>     coro_jumpAttackUID(uint64_t);
        corof::eval_awaiter<bool>     coro_trackAttackUID(uint64_t);
        corof::eval_awaiter<bool>     coro_inDCCastRange(uint64_t, DCCastRange);
        corof::eval_awaiter<std::optional<SDHealth>> coro_queryHealth(uint64_t);

    public:
        static bool isPet(uint64_t);

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
};
