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
        bool TrackAttack();
        bool FollowMaster();

    protected:
        bool TrackUID(uint64_t);
        bool AttackUID(uint64_t, int);

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
        void On_MPK_EXP           (const MessagePack &);
        void On_MPK_ATTACK        (const MessagePack &);
        void On_MPK_ACTION        (const MessagePack &);
        void On_MPK_OFFLINE       (const MessagePack &);
        void On_MPK_UPDATEHP      (const MessagePack &);
        void On_MPK_METRONOME     (const MessagePack &);
        void On_MPK_MAPSWITCH     (const MessagePack &);
        void On_MPK_NOTIFYDEAD    (const MessagePack &);
        void On_MPK_BADACTORPOD   (const MessagePack &);
        void On_MPK_CHECKMASTER   (const MessagePack &);
        void On_MPK_DEADFADEOUT   (const MessagePack &);
        void On_MPK_NOTIFYNEWCO   (const MessagePack &);
        void On_MPK_QUERYCORECORD (const MessagePack &);
        void On_MPK_QUERYLOCATION (const MessagePack &);

    protected:
        void OperateAM(const MessagePack &);

    protected:
        void ReportCORecord(uint64_t);

    protected:
        bool MoveOneStep(int, int);

    protected:
        void CheckCurrTarget();

    protected:
        int FindPathMethod();

    protected:
        void RandomDrop();

    protected:
        void CheckFriend(uint64_t, const std::function<void(int)> &);

    protected:
        bool MoveOneStepAStar   (int, int, std::function<void()> /* fnOnError */);
        bool MoveOneStepGreedy  (int, int, std::function<void()> /* fnOnError */);
        bool MoveOneStepCombine (int, int, std::function<void()> /* fnOnError */);
        bool MoveOneStepNeighbor(int, int, std::function<void()> /* fnOnError */);

    protected:
        bool CanMove();
        bool CanAttack();

    protected:
        void CheckMaster();

    protected:
        virtual bool GoDie();
        virtual bool GoGhost();

    protected:
        virtual void CreateBvTree();

    protected:
        virtual bvnode_ptr BvTree_GetMasterUID();
        virtual bvnode_ptr BvTree_FollowMaster();
        virtual bvnode_ptr BvTree_LocateUID(bvarg_ptr);
        virtual bvnode_ptr BvTree_LocateMaster();
};
