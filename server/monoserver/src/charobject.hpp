/*
 * =====================================================================================
 *
 *       Filename: charobject.hpp
 *        Created: 04/10/2016 12:05:22
 *  Last Modified: 04/13/2016 20:01:31
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

#include <list>
#include <vector>

#include "activeobject.hpp"
#include "servermap.hpp"

enum RangeType: uint8_t{
    RANGE_VIEW,
    RANGE_MAP,
    RANGE_SERVER,

    RANGE_ATTACK,
    RANGE_TRACETARGET,
};


// define of directioin
//
//               0
//            7     1
//          6    +-----> 2
//            5  |  3
//               |
//               V
//               4
//
enum Direction: uint8_t{
    DIR_UP          = 0,
    DIR_DOWN        = 4,
    DIR_LEFT        = 6,
    DIR_RIGHT       = 2,
    DIR_UPLEFT      = 7,
    DIR_UPRIGHT     = 1,
    DIR_DOWNLEFT    = 5,
    DIR_DOWNRIGHT   = 3,
};

#pragma pack(push, 1)
typedef struct{
    uint8_t     Level;
    uint16_t    HP;
    uint16_t    MP;
    uint16_t    MaxHP;
    uint16_t    MaxMP;
    uint16_t    Weight;
    uint16_t    MaxWeight;
    uint32_t    Exp;
    uint32_t    MaxExp;

    uint8_t     WearWeight;
    uint8_t     MaxWearWeight;
    uint8_t     HandWeight;
    uint8_t     MaxHandWeight;

    uint16_t    DC;
    uint16_t    MC;
    uint16_t    SC;
    uint16_t    AC;
    uint16_t    MAC;

    uint16_t    Water;
    uint16_t    Fire;
    uint16_t    Wind;
    uint16_t    Light;
    uint16_t    Earth;
}OBJECTABILITY;

typedef struct{
    uint16_t    HP;
    uint16_t    MP;
    uint16_t    HIT;
    uint16_t    SPEED;
    uint16_t    AC;
    uint16_t    MAC;
    uint16_t    DC;
    uint16_t    MC;
    uint16_t    SC;
    uint16_t    AntiPoison;
    uint16_t    PoisonRecover;
    uint16_t    HealthRecover;
    uint16_t    SpellRecover;
    uint16_t    AntiMagic;
    uint8_t     Luck;
    uint8_t     UnLuck;
    uint8_t     WeaponStrong;
    uint16_t    HitSpeed;
}OBJECTADDABILITY;
#pragma pack(pop)

class CharObject: public ActiveObject
{
    private:
        // define some shortcuts for internal use only
        // let's use std::list here instead of std::forward_list
        // std::list is a double linked list and support list.erase()
        // while forward_list only support list.erase_after()
        //
        // memery: list use three times of memery as to forward_list
        // time  : almost the same
        using ObjectRecord      = std::tuple<uint32_t, uint32_t>;
        using ObjectRecordList  = std::list<ObjectRecord>;
        using CacheObjectRecord = std::tuple<uint8_t, uint8_t, uint32_t, uint32_t>;
        const ObjectRecord EMPTYOBJECTRECORD {0, 0};

    public:
        CharObject();
        ~CharObject();

    public:
        bool Active()
        {
            return !State(STATE_DEAD) && !State(STATE_GHOST);
        }

    public:
        // this function give an advice for master object
        // since it returns ID rather than object pointer
        // bool Master(uint32_t *pUID = nullptr, uint32_t *pAddTime = nullptr)
        // {
        //     if(m_Master == EMPTYOBJECTRECORD){ return false; }
        //     extern MonoServer *g_MonoServer;
        //     if(auto pGuard = g_MonoServer->CheckOut<CharObject>(
        //                 std::get<0>(m_Master), std::get<1>(m_Master))){
        //         if(pUID    ){ *pUID     = std::get<0>(m_Master); }
        //         if(pAddTime){ *pAddTime = std::get<1>(m_Master); }
        //         return true;
        //     }
        //
        //     m_Master = EMPTYOBJECTRECORD;
        //     return false;
        // }

    public:
        int X()
        {
            return m_CurrX;
        }

        int Y()
        {
            return m_CurrY;
        }

        int R()
        {
            return m_R;
        }

        uint32_t MapID()
        {
            return (m_Map ? m_Map->ID() : 0);
        }

    protected:
        int m_R;

    public:
        virtual bool Follow(CharObject *, bool)
        {
            return true;
        }

        virtual bool Attack(CharObject *) = 0;

        virtual bool Friend(const CharObject *) const = 0;
        virtual bool Operate() = 0;
        virtual int  NameColorType() = 0;
        virtual const char *CharName() = 0;

        virtual bool Mode(uint8_t)
        {
            return true;
        }

    public:
        uint8_t GetBack()
        {
            switch (m_Direction){
                case DIR_UP       : return DIR_DOWN;
                case DIR_DOWN     : return DIR_UP;
                case DIR_LEFT     : return DIR_RIGHT;
                case DIR_RIGHT    : return DIR_LEFT;
                case DIR_UPLEFT   : return DIR_DOWNRIGHT;
                case DIR_UPRIGHT  : return DIR_DOWNLEFT;
                case DIR_DOWNLEFT : return DIR_UPRIGHT;
                case DIR_DOWNRIGHT: return DIR_UPLEFT;
                default: break;
            }

            // make the compiler happy
            return m_Direction;
        }

    protected:
        uint16_t m_ObjectType;

    public:
        // test function
        virtual bool Type( uint8_t) const = 0;
        virtual bool State(uint8_t) const = 0;

    public:
        virtual int Range(uint8_t) = 0;

    protected:
        int     m_CurrX;
        int     m_CurrY;
        int     m_Direction;
        int     m_Event;

        ServerMap*                  m_Map;

        ObjectRecordList m_VisibleObjectList;
        ObjectRecordList m_VisibleItemList;
        ObjectRecordList m_VisibleEventList;

        OBJECTABILITY               m_Ability;
        OBJECTABILITY               m_WAbility;
        OBJECTADDABILITY            m_AddAbility;

        std::array<uint8_t,  256>    m_StateAttrV;
        std::array<uint32_t, 256>    m_StateTimeV;

        ObjectRecord m_Master;
        ObjectRecord m_Target;

    public:
        bool SetTarget(uint32_t nUID, uint32_t nAddTime)
        {
            std::get<0>(m_Target) = nUID;
            std::get<1>(m_Target) = nAddTime;
            return true;
        }

        bool Target(uint32_t *pUID = nullptr, uint32_t *pAddTime = nullptr)
        {
            if(m_Target != EMPTYOBJECTRECORD){
                if(pUID    ){ *pUID     = std::get<0>(m_Target); }
                if(pAddTime){ *pAddTime = std::get<1>(m_Target); }
                return true;
            }
            return false;
        }

    public:

        bool    GetNextPosition(int nSX, int nSY, int nDir, int nDistance, int& nX, int& nY);
        int     GetNextDirection(int nStartX, int nStartY, int nTargetX, int nTargetY);
        void    TurnTo(int nDir);
        bool    TurnXY(int nX, int nY, int nDir);
        bool    WalkXY(int nX, int nY, int nDir);
        bool    RunTo(int nDir);
        bool    RunXY(int nX, int nY, int nDir);
        bool    HitXY(uint16_t wIdent, int nX, int nY, int nDir, int nHitStyle);

        void    Disappear();
        void    MakeGhost();

        // bool    DropItem(uint32_t, uint32_t, int);


        void    SpaceMove(int nX, int nY, ServerMap *);
        void    Die();

    protected:
        std::string m_Name;

    public:
        void NextLocation(int *, int *, int);

        uint8_t Direction(int, int);

        uint8_t Direction()
        {
            return m_Direction;
        }

    public:

        // this function make a cache for object/item/event view by objects
        // we'll repeat to call this function to update it by time, rather
        // than ``update" it by in/out-sight event driven.
        //
        // TODO & TBD for SearchViewRange()
        //
        // when check objects inside the area, do we need to lock all the gird
        // at one time and update the object records in the range, or just do
        // it one by one?
        // --------------------------------------------------------------------
        // method-1                        |   method-2:
        //     for_each(cell(i, j))        |       for_each(cell(i, j))
        //     do                          |       do 
        //         lock(cell(i, j))        |           lock(cell(i, j))
        //         update(i, j)            |       done
        //         unlock(cell(i, j))      |   
        //     done                        |       for_each(cell(i, j))
        //                                 |           update(i, j)
        //                                 |       done
        //                                 |     
        //                                 |       for_each(cell(i, j))
        //                                 |       do 
        //                                 |           unlock(cell(i, j))
        //                                 |       done
        // --------------------------------------------------------------------
        //
        // currently I use method-1, method-2 guarantees consistancy inside the 
        // function, however, even we get the 100% correct information, when out
        // of this function, this correctness immidately becomes vulnerable.
        //  
        virtual void SearchViewRange() = 0;
};
