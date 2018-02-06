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
#include "charobject.hpp"
#include "monsterrecord.hpp"

typedef struct stMONSTERITEMINFO
{
    int     MonsterIndex;
    int     Type;
    int     Chance;
    int     Count;

    stMONSTERITEMINFO(int nMonsterIndex = -1)
        : MonsterIndex(nMonsterIndex)
    {}
}MONSTERITEMINFO;

typedef struct stMONSTERRACEINFO
{
    int     Index;
    int     Race;
    int     LID;
    int     Undead;
    int     Level;
    int     HP;
    int     MP;
    int     AC;
    int     MAC;
    int     DC;
    int     AttackSpead;
    int     WalkSpead;
    int     Spead;
    int     Hit;
    int     ViewRange;
    int     RaceIndex;
    int     Exp;
    int     Escape;
    int     Water;
    int     Fire;
    int     Wind;
    int     Light;
    int     Earth;

    std::string Name;
    std::vector<MONSTERITEMINFO> ItemV;

    stMONSTERRACEINFO(int nIndex = -1)
        : Index(nIndex)
        , Name("")
    {}
}MONSTERRACEINFO;

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
        };

    protected:
        const uint32_t m_MonsterID;

    protected:
        uint32_t m_MasterUID;

    protected:
        const MonsterRecord &m_MonsterRecord;

    protected:
        AStarCache m_AStarCache;

    public:
        Monster(uint32_t,               // monster id
                ServiceCore *,          // service core
                ServerMap *,            // server map
                int,                    // map x
                int,                    // map y
                int,                    // direction
                uint8_t,                // life cycle state
                uint32_t);              // master uid

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
       uint32_t MasterUID()
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
        bool TrackUID(uint32_t);
        bool AttackUID(uint32_t, int);

    protected:
        bool DCValid(int, bool);
        bool InRange(int, int, int);

    protected:
        void AddTarget(uint32_t);
        void RemoveTarget(uint32_t);

    protected:
        bool StruckDamage(const DamageNode &);

    protected:
        DamageNode GetAttackDamage(int);

    private:
        void On_MPK_EXP(const MessagePack &, const Theron::Address &);
        void On_MPK_ATTACK(const MessagePack &, const Theron::Address &);
        void On_MPK_ACTION(const MessagePack &, const Theron::Address &);
        void On_MPK_OFFLINE(const MessagePack &, const Theron::Address &);
        void On_MPK_UPDATEHP(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_MAPSWITCH(const MessagePack &, const Theron::Address &);
        void On_MPK_NOTIFYDEAD(const MessagePack &, const Theron::Address &);
        void On_MPK_BADACTORPOD(const MessagePack &, const Theron::Address &);
        void On_MPK_NOTIFYNEWCO(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYCORECORD(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYLOCATION(const MessagePack &, const Theron::Address &);

    protected:
        void OperateAM(const MessagePack &, const Theron::Address &);

    protected:
        void ReportCORecord(uint32_t);

    protected:
        bool MoveOneStep(int, int);

    protected:
        void CheckCurrTarget();

    protected:
        int FindPathMethod();

    protected:
        void RandomDrop();

    public:
        InvarData GetInvarData() const;

    protected:
        void CheckFriend(uint32_t, const std::function<void(int)> &);

    protected:
        bool MoveOneStepAStar  (int, int);
        bool MoveOneStepGreedy (int, int);
        bool MoveOneStepCombine(int, int);

    protected:
        bool CanMove();
        bool CanAttack();

    protected:
        std::array<PathFind::PathNode, 3> GetChaseGrid(int, int);

    protected:
        virtual bool GoDie();
        virtual bool GoGhost();
        virtual bool GoSuicide();
};
