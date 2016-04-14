/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 04/10/2016 02:32:45 AM
 *  Last Modified: 04/14/2016 00:08:42
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
    public:
        friend class MonoServer;

    protected:
        Monster(uint32_t, uint32_t, uint32_t);

    public:
        ~Monster();

    private:
        // shortcuts for internal use only
        using ObjectRecord     = std::tuple<uint32_t, uint32_t>;
        using ObjectRecordList = std::list<ObjectRecord>;
        const ObjectRecord EMPTYOBJECTRECORD {0, 0};


    protected:
        uint32_t m_MonsterIndex;

    public:
        // type test function
        virtual bool Type(uint8_t);
        virtual bool State(uint8_t);

        virtual bool SetType(uint8_t, bool);
        virtual bool SetState(uint8_t, bool);

        virtual uint32_t NameColor();
        virtual const char *CharName();

        virtual int Range(uint8_t);

        virtual bool Attack(CharObject *);
        virtual bool Follow(CharObject *, bool);
        virtual bool Operate();

    public:
        virtual void SearchViewRange();

    private:
        // for monster std::list<std::tuple<uint32_t, uint32_t>> is good enough
        // for monster we don't need to differ show up/off of an object
        ObjectRecordList m_VisibleObjectList;

    public:
        virtual bool Friend(CharObject *);

    public:
        bool RandomWalk();
};
