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
#include "bvtree.hpp"
#include "charobject.hpp"
#include "monsterrecord.hpp"

class Monster final: public CharObject
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
            FPMETHOD_GREEDY,
            FPMETHOD_COMBINE,
            FPMETHOD_NEIGHBOR,
        };

    protected:
        friend class CharObject;

    protected:
        const uint32_t m_MonsterID;

    protected:
        uint64_t m_MasterUID;

    protected:
        const MonsterRecord &m_MonsterRecord;

    protected:
        AStarCache m_AStarCache;

    protected:
        bvnode_ptr m_BvTree;

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
       uint32_t MonsterID() const
       {
           return m_MonsterID;
       }

    protected:
       // don't expose it to public
       // master may change by time or by magic
       uint64_t MasterUID() const
       {
           return m_MasterUID;
       }

    protected:
        void SearchViewRange();
        bool Update();

    protected:
        bool RandomMove();
        bool FollowMaster(std::function<void()>, std::function<void()>);

    protected:
        void RecursiveCheckInViewTarget(size_t, std::function<void(uint64_t)>);
        void SearchNearestTarget(std::function<void(uint64_t)>);

    protected:
        void TrackUID      (uint64_t, int, std::function<void()>, std::function<void()>);
        void AttackUID     (uint64_t, int, std::function<void()>, std::function<void()>);
        void TrackAttackUID(uint64_t,      std::function<void()>, std::function<void()>);

    protected:
        bool DCValid(int, bool);
        bool InRange(int, int, int);

    protected:
        void AddTarget(uint64_t);
        void RemoveTarget(uint64_t);

    protected:
        bool StruckDamage(const DamageNode &);

    protected:
        DamageNode GetAttackDamage(int);

    private:
        void On_MPK_EXP             (const MessagePack &);
        void On_MPK_ATTACK          (const MessagePack &);
        void On_MPK_ACTION          (const MessagePack &);
        void On_MPK_OFFLINE         (const MessagePack &);
        void On_MPK_UPDATEHP        (const MessagePack &);
        void On_MPK_METRONOME       (const MessagePack &);
        void On_MPK_MAPSWITCH       (const MessagePack &);
        void On_MPK_NOTIFYDEAD      (const MessagePack &);
        void On_MPK_BADACTORPOD     (const MessagePack &);
        void On_MPK_CHECKMASTER     (const MessagePack &);
        void On_MPK_QUERYMASTER     (const MessagePack &);
        void On_MPK_DEADFADEOUT     (const MessagePack &);
        void On_MPK_NOTIFYNEWCO     (const MessagePack &);
        void On_MPK_QUERYCORECORD   (const MessagePack &);
        void On_MPK_QUERYLOCATION   (const MessagePack &);
        void On_MPK_QUERYNAMECOLOR  (const MessagePack &);
        void On_MPK_QUERYFRIENDTYPE (const MessagePack &);
        void On_MPK_QUERYFINALMASTER(const MessagePack &);

    protected:
        void OperateAM(const MessagePack &);

    protected:
        void ReportCORecord(uint64_t);

    protected:
        bool MoveOneStep(int, int, std::function<void()>, std::function<void()>);

    protected:
        void GetProperTarget(std::function<void(uint64_t)>);

    protected:
        int FindPathMethod();

    protected:
        void RandomDrop();

    protected:
        void QueryFriendType(uint64_t, uint64_t, std::function<void(int)>);
        void CheckFriendType(uint64_t, std::function<void(int)>) override;

    private:
        void CheckFriendType_AsGuard      (uint64_t, std::function<void(int)>);
        void CheckFriendType_CtrlByPlayer (uint64_t, std::function<void(int)>);
        void CheckFriendType_CtrlByMonster(uint64_t, std::function<void(int)>);

    protected:
        void CheckFriend(uint64_t, const std::function<void(int)> &);
        void QueryMaster(uint64_t, std::function<void(uint64_t)>);

    protected:
        bool MoveOneStepAStar   (int, int, std::function<void()>, std::function<void()>);
        bool MoveOneStepGreedy  (int, int, std::function<void()>, std::function<void()>);
        bool MoveOneStepCombine (int, int, std::function<void()>, std::function<void()>);
        bool MoveOneStepNeighbor(int, int, std::function<void()>, std::function<void()>);

    public:
        uint64_t Activate() override;

    protected:
        bool CanMove();
        bool CanAttack();

    protected:
        virtual bool GoDie();
        virtual bool GoGhost();

    protected:
        virtual void CreateBvTree();

    protected:
        virtual bvnode_ptr BvNode_HasMaster();
        virtual bvnode_ptr BvNode_GetMasterUID(bvarg_ref);
        virtual bvnode_ptr BvNode_FollowMaster();
        virtual bvnode_ptr BvNode_LocateUID(bvarg_ref, bvarg_ref);
        virtual bvnode_ptr BvNode_RandomMove();
        virtual bvnode_ptr BvNode_RandomTurn();
        virtual bvnode_ptr BvNode_AttackUID(bvarg_ref, bvarg_ref);
        virtual bvnode_ptr BvNode_MoveForwardOneStep();
        virtual bvnode_ptr BvNode_MoveOneStep(bvarg_ref);
        virtual bvnode_ptr BvNode_GetProperTarget(bvarg_ref);
        virtual bvnode_ptr BvNode_TrackAttackUID(bvarg_ref);

    public:
        static bool IsPet(uint64_t);
        static bool IsGuard(uint64_t);
};
