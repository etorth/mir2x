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
#include <functional>
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
            uint32_t MapID;
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
        const uint32_t m_monsterID;

    protected:
        uint64_t m_masterUID;

    protected:
        const MonsterRecord &m_monsterRecord;

    protected:
        AStarCache m_AStarCache;

    protected:
        corof::long_jmper m_updateCoro;

    public:
        Monster(uint32_t,               // monster id
                ServiceCore *,          // service core
                ServerMap *,            // server map
                int,                    // map x
                int,                    // map y
                int,                    // direction
                uint64_t);              // master uid

    public:
        ~Monster() = default;

    public:
       uint32_t monsterID() const
       {
           return m_monsterID;
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
        corof::long_jmper updateCoroFunc();

    protected:
        bool randomMove();
        bool randomTurn();
        void followMaster(std::function<void()>, std::function<void()>);

    protected:
        void RecursiveCheckInViewTarget(size_t, std::function<void(uint64_t)>);
        void SearchNearestTarget(std::function<void(uint64_t)>);

    protected:
        virtual void trackUID      (uint64_t, int, std::function<void()>, std::function<void()>);
        virtual void attackUID     (uint64_t, int, std::function<void()>, std::function<void()>);
        virtual void trackAttackUID(uint64_t,      std::function<void()>, std::function<void()>);

    protected:
        bool DCValid(int, bool);
        bool InRange(int, int, int);

    protected:
        virtual void SetTarget(uint64_t);
        virtual void RemoveTarget(uint64_t);

    protected:
        bool StruckDamage(const DamageNode &);

    protected:
        DamageNode GetAttackDamage(int);

    private:
        void on_MPK_EXP             (const MessagePack &);
        void on_MPK_MISS            (const MessagePack &);
        void on_MPK_ATTACK          (const MessagePack &);
        void on_MPK_ACTION          (const MessagePack &);
        void on_MPK_OFFLINE         (const MessagePack &);
        void on_MPK_UPDATEHP        (const MessagePack &);
        void on_MPK_METRONOME       (const MessagePack &);
        void on_MPK_MAPSWITCH       (const MessagePack &);
        void on_MPK_MASTERKILL      (const MessagePack &);
        void on_MPK_MASTERHITTED    (const MessagePack &);
        void on_MPK_NOTIFYDEAD      (const MessagePack &);
        void on_MPK_BADACTORPOD     (const MessagePack &);
        void on_MPK_CHECKMASTER     (const MessagePack &);
        void on_MPK_QUERYMASTER     (const MessagePack &);
        void on_MPK_DEADFADEOUT     (const MessagePack &);
        void on_MPK_NOTIFYNEWCO     (const MessagePack &);
        void on_MPK_QUERYCORECORD   (const MessagePack &);
        void on_MPK_QUERYLOCATION   (const MessagePack &);
        void on_MPK_QUERYNAMECOLOR  (const MessagePack &);
        void on_MPK_QUERYFRIENDTYPE (const MessagePack &);
        void on_MPK_QUERYFINALMASTER(const MessagePack &);

    protected:
        void operateAM(const MessagePack &);

    protected:
        void reportCO(uint64_t);

    protected:
        bool MoveOneStep(int, int, std::function<void()>, std::function<void()>);

    protected:
        void GetProperTarget(std::function<void(uint64_t)>);

    protected:
        int FindPathMethod();

    protected:
        void randomDrop();

    protected:
        void QueryFriendType(uint64_t, uint64_t, std::function<void(int)>);
        void checkFriend(uint64_t, std::function<void(int)>) override;

    private:
        void checkFriend_AsGuard      (uint64_t, std::function<void(int)>);
        void checkFriend_CtrlByPlayer (uint64_t, std::function<void(int)>);
        void checkFriend_CtrlByMonster(uint64_t, std::function<void(int)>);

    protected:
        void QueryMaster(uint64_t, std::function<void(uint64_t)>);

    protected:
        bool MoveOneStepAStar   (int, int, std::function<void()>, std::function<void()>);
        bool MoveOneStepDStar   (int, int, std::function<void()>, std::function<void()>);
        bool MoveOneStepGreedy  (int, int, std::function<void()>, std::function<void()>);
        bool MoveOneStepCombine (int, int, std::function<void()>, std::function<void()>);
        bool MoveOneStepNeighbor(int, int, std::function<void()>, std::function<void()>);

    public:
        uint64_t activate() override;

    protected:
        bool canMove();
        bool canAttack();

    protected:
        virtual bool goDie();
        virtual bool goGhost();

    protected:
        corof::long_jmper::eval_op<bool>     coro_randomMove();
        corof::long_jmper::eval_op<bool>     coro_moveForward();
        corof::long_jmper::eval_op<bool>     coro_followMaster();
        corof::long_jmper::eval_op<uint64_t> coro_getProperTarget();
        corof::long_jmper::eval_op<bool>     coro_trackAttackUID(uint64_t);

    public:
        static bool isPet(uint64_t);
        static bool isGuard(uint64_t);

    private:
        int walkWait() const
        {
            return DBCOM_MONSTERRECORD(monsterID()).walkWait;
        }

        int attackWait() const
        {
            return DBCOM_MONSTERRECORD(monsterID()).walkWait;
        }

    protected:
        template<size_t N> bool checkMonsterName(const char8_t (&name)[N]) const
        {
            return monsterID() == DBCOM_MONSTERID(name);
        }

        template<size_t N> void checkMonsterNameEx(const char8_t (&name)[N]) const
        {
            if(!checkMonsterName(name)){
                throw fflerror("invalid monster name: %s", to_cstr(DBCOM_MONSTERRECORD(monsterID()).name));
            }
        }
};
