/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 04/10/2016 02:32:45 AM
 *  Last Modified: 04/10/2016 23:05:15
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

    std::string     Name;
    std::vector<MONSTERITEMINFO> ItemV;

    stMONSTERRACEINFO(int nIndex = -1)
        : Index(nIndex)
        , Name("")
    {}

}MONSTERRACEINFO;

class Monster: public CharObject
{
    private:
        // shortcuts for internal use only
        using ObjectRecord     = std::tuple<uint32_t, uint32_t>;
        using ObjectRecordList = std::list<ObjectRecord>;
        const ObjectRecord EMPTYOBJECTRECORD {0, 0};

    public:
        Monster();
        ~Monster();

    public:
        // type test function
        virtual bool Type(uint8_t) const;
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
        virtual bool Friend(const CharObject *);

    public:
        bool RandomWalk();
};
