/*
 * =====================================================================================
 *
 *       Filename: charobject.hpp
 *        Created: 04/10/2016 12:05:22
 *  Last Modified: 05/30/2016 12:21:32
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

enum _FriendType: uint8_t{
    FRIEND_HUMAN,
    FRIEND_ANIMAL,
};

enum _RangeType: uint8_t{
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
enum _Direction: uint8_t{
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
    protected:
        Theron::Address m_RMAddress;

    public:
        CharObject();
        ~CharObject();

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

        int R()
        {
            return m_R;
        }

    protected:
        int m_R;
        uint32_t m_MapID;

    public:
        virtual bool Follow(CharObject *, bool)
        {
            return true;
        }

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
        virtual int Range(uint8_t) = 0;

    protected:
        int     m_CurrX;
        int     m_CurrY;
        int     m_Direction;
        int     m_Event;

        OBJECTABILITY               m_Ability;
        OBJECTABILITY               m_WAbility;
        OBJECTADDABILITY            m_AddAbility;

        std::array<uint8_t,  256>    m_StateAttrV;
        std::array<uint32_t, 256>    m_StateTimeV;

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


        void    Die();

        void Move();

    public:
        virtual bool Update() = 0;

    protected:
        std::string m_Name;

    public:
        void NextLocation(int *, int *, int);

        uint8_t Direction(int, int);

        uint8_t Direction()
        {
            return m_Direction;
        }

    protected:
        void InnSetR(int nR)
        {
            m_R = nR;
        }

        void InnSetMapID(uint32_t nMapID)
        {
            m_MapID = nMapID;
        }

        void InnSetLocation(const Theron::Address &rstAddress, int nX, int nY)
        {
            m_CurrX = nX;
            m_CurrY = nY;
            m_RMAddress = rstAddress;
        }

    public:
        // TODO & TBD
        // do I need to check whether it's proper?
        void Locate(uint32_t nMapID, int nMapX, int nMapY)
        {
            if(AccessCheck()){
                m_MapID = nMapID;
                m_CurrX = nMapX;
                m_CurrY = nMapY;
            }
        }

        void Locate(const Theron::Address &rstAddr)
        {
            if(AccessCheck()){
                m_RMAddress = rstAddr;
            }
        }

        void ResetR(int nR)
        {
            if(AccessCheck()){
                InnSetR(nR);
            }
        }
};
