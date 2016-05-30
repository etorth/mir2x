/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 04/10/2016 02:32:45 AM
 *  Last Modified: 05/30/2016 12:37:07
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
#include "charobject.hpp"

enum MonsterType: uint32_t{
    MONSTER_DEER,
};

typedef struct stMONSTERITEMINFO{
    int     MonsterIndex;
    int     Type;
    int     Chance;
    int     Count;

    stMONSTERITEMINFO(int nMonsterIndex = -1)
        : MonsterIndex(nMonsterIndex)
    {}
}MONSTERITEMINFO;

typedef struct stMONSTERRACEINFO{
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

    std::string     Name;
    std::vector<MONSTERITEMINFO> ItemV;

    stMONSTERRACEINFO(int nIndex = -1)
        : Index(nIndex)
        , Name("")
    {}
}MONSTERRACEINFO;

class MonoServer;
class Monster: public CharObject
{
    private:
        typedef struct _ActorRecord{
            Theron::Address PodAddress;       // void to use Address...
            uint32_t        UID;
            uint32_t        AddTime;
            int             X;
            int             Y;
            uint8_t         FriendType;

            _ActorRecord(const Theron::Address &stAddr = Theron::Address::Null(),
                    uint32_t nUID     = 0,
                    uint32_t nAddTime = 0,
                    int nX = 0,
                    int nY = 0,
                    uint8_t nFriendType = FRIEND_HUMAN)
                : PodAddress(stAddr)
                  , UID(nUID)
                  , AddTime(nAddTime)
                  , X(nX)
                  , Y(nY)
                  , FriendType(nFriendType)
            {}

            bool Valid()
            {
                return PodAddress == Theron::Address::Null() || UID == 0 || AddTime == 0;
            }

            bool operator == (const _ActorRecord &rstRecord)
            {
                return true
                    && PodAddress == rstRecord.PodAddress
                    && UID == rstRecord.UID
                    && AddTime == rstRecord.AddTime;
            }

        }ActorRecord;

    protected:
        uint32_t                 m_MonsterID;
        std::vector<ActorRecord> m_NeighborV;

    public:
        Monster(uint32_t);
        ~Monster();

    public:
        bool Type(uint8_t);
        bool State(uint8_t);

        bool ResetType(uint8_t, bool);
        bool ResetState(uint8_t, bool);

    public:
        uint32_t NameColor();
        const char *CharName();

        int Range(uint8_t);

        bool Attack(CharObject *);
        bool Follow(CharObject *, bool);

    public:
        void SearchViewRange();

    public:

        bool ReportMove(int, int);
        int Speed()
        {
            return 5;
        }

        // TODO
        bool m_FreezeWalk;
        bool Update();

    public:
        void SpaceMove(const char *, int, int);
        bool RandomWalk();

    private:
        void On_MPK_HI(const MessagePack &, const Theron::Address &);
        void On_MPK_MOVEOK(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_LOCATIION(const MessagePack &, const Theron::Address &);
        void On_MPK_MASTERPERSONA(const MessagePack &, const Theron::Address &);

    protected:
        void Operate(const MessagePack &, const Theron::Address &);
};
