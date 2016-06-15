/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 04/10/2016 02:32:45 AM
 *  Last Modified: 06/15/2016 01:31:02
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
            int             FriendType;

            _ActorRecord(const Theron::Address &stAddr = Theron::Address::Null(),
                    uint32_t nUID     = 0,
                    uint32_t nAddTime = 0,
                    int nX = 0,
                    int nY = 0,
                    int nFriendType = FRIEND_HUMAN)
                : PodAddress(stAddr)
                , UID(nUID)
                , AddTime(nAddTime)
                , X(nX)
                , Y(nY)
                , FriendType(nFriendType)
            {}

            bool Valid()
            {
                return PodAddress && UID && AddTime;
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
        bool m_FreezeWalk;
        uint32_t m_MonsterID;
        std::vector<ActorRecord> m_ActorRecordV;

    public:
        Monster(uint32_t);
        ~Monster();

    public:
        uint8_t Type(uint8_t);
        uint8_t State(uint8_t);

        bool ResetType(uint8_t, uint8_t);
        bool ResetState(uint8_t, uint8_t);

    public:
        uint32_t NameColor();
        const char *CharName();

        int Range(uint8_t);

    public:
        void SearchViewRange();

    public:

        bool ReportMove(int, int);
        int Speed()
        {
            return 20;
        }

        // TODO
        bool Update();

    public:
        void SpaceMove(const char *, int, int);
        bool RandomWalk();

    private:
        bool UpdateLocation();

    private:
        void On_MPK_HI(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_UPDATECOINFO(const MessagePack &, const Theron::Address &);

    protected:
        void Operate(const MessagePack &, const Theron::Address &);

    protected:
        void ReportCORecord(uint32_t);

#ifdef MIR2X_DEBUG
    protected:
        const char *ClassName()
        {
            return "Monster";
        }
#endif
};
