/*
 * =====================================================================================
 *
 *       Filename: charobject.hpp
 *        Created: 04/10/2016 12:05:22
 *  Last Modified: 03/22/2017 17:24:11
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

#include "servermap.hpp"
#include "servicecore.hpp"
#include "activeobject.hpp"

enum _FriendType: uint8_t
{
    FRIEND_HUMAN,
    FRIEND_ANIMAL,
    FRIEND_NEUTRAL,
};

enum _RangeType: uint8_t
{
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
//          6    +--> 2
//            5  |  3
//               V
//               4
//
enum _Direction: int
{
    DIR_UP          = 0,
    DIR_DOWN        = 4,
    DIR_LEFT        = 6,
    DIR_RIGHT       = 2,
    DIR_UPLEFT      = 7,
    DIR_UPRIGHT     = 1,
    DIR_DOWNLEFT    = 5,
    DIR_DOWNRIGHT   = 3,
    DIR_UNKNOWN     = 8,
};

#pragma pack(push, 1)
typedef struct
{
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

typedef struct
{
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
    protected:
        ServiceCore *m_ServiceCore;
        ServerMap   *m_Map;

    protected:
        int m_CurrX;
        int m_CurrY;
        int m_Direction;

    protected:
        OBJECTABILITY       m_Ability;
        OBJECTABILITY       m_WAbility;
        OBJECTADDABILITY    m_AddAbility;

    public:
        CharObject(ServiceCore *,       // service core
                ServerMap *,            // server map
                int,                    // map x
                int,                    // map y
                int,                    // direction
                uint8_t,                // life cycle state
                uint8_t);               // action state
       ~CharObject() = default;

    public:
        bool Active()
        {
            return !State(STATE_DEAD) && !State(STATE_PHANTOM);
        }

        virtual int Speed() = 0;

    public:
        int X()
        {
            return m_CurrX;
        }

        int Y()
        {   
            return m_CurrY;
        }

        int Direction()
        {
            return m_Direction;
        }

        uint32_t MapID()
        {
            return m_Map ? m_Map->ID() : 0;
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
                default           : return DIR_UNKNOWN;
            }
        }

    public:
        int Action();

    public:
        virtual int  Range(uint8_t) = 0;
        virtual bool Update()       = 0;

    public:
        uint8_t Direction(int, int);
        bool    NextLocation(int *, int *, int);

    protected:
        virtual void ReportCORecord(uint32_t) = 0;

    protected:
        void DispatchAction();
};
