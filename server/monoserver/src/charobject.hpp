/*
 * =====================================================================================
 *
 *       Filename: charobject.hpp
 *        Created: 04/10/2016 12:05:22
 *  Last Modified: 06/09/2016 18:25:34
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
#include "activeobject.hpp"

enum _FriendType: uint8_t{
    FRIEND_HUMAN,
    FRIEND_ANIMAL,
    FRIEND_NEUTRAL,
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
//          6    +--> 2
//            5  |  3
//               V
//               4
//
enum _Direction: int{
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
        enum QueryType: int{
            QUERY_NA,
            QUERY_OK,
            QUERY_ERROR,
            QUERY_PENDING,
        };

    protected:
        Theron::Address m_EmptyAddress; // to return ref to null
        Theron::Address m_RMAddress;    //
        Theron::Address m_MapAddress;   //
        Theron::Address m_SCAddress;    //

        int m_MapAddressQuery;          //
        int m_SCAddressQuery;           //

    protected:
        uint32_t m_MapID;

    protected:
        int     m_CurrX;
        int     m_CurrY;
        int     m_R;
        int     m_Direction;

        OBJECTABILITY       m_Ability;
        OBJECTABILITY       m_WAbility;
        OBJECTADDABILITY    m_AddAbility;

    protected:
        std::string m_Name;

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

        int Direction()
        {
            return m_Direction;
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

    public:
        virtual int  Range(uint8_t) = 0;
        virtual bool Update()       = 0;

    public:
        uint8_t Direction(int, int);
        void    NextLocation(int *, int *, int);

    protected:
        void InnResetR(int nR)
        {
            m_R = nR;
        }

        void InnResetMapID(uint32_t nMapID)
        {
            m_MapID = nMapID;
        }

        void InnLocate(const Theron::Address &rstRMAddr)
        {
            m_RMAddress = rstRMAddr;
        }

        void InnLocate(uint32_t nMapID, int nX, int nY)
        {
            m_MapID = nMapID;
            m_CurrX = nX;
            m_CurrY = nY;
        }

    public:
        template<typename... T> void Locate(T&&... stT)
        {
            if(AccessCheck()){
                InnLocate(std::forward<T>(stT)...);
            }
        }

        template<typename... T> void ResetMapID(T&&... stT)
        {
            if(AccessCheck()){
                InnResetMapID(std::forward<T>(stT)...);
            }
        }

        template<typename... T> void ResetR(T&&... stT)
        {
            if(AccessCheck()){
                InnResetR(std::forward<T>(stT)...);
            }
        }

    public:
        const Theron::Address &QueryAddress()
        {
            if(m_RMAddress ){ return m_RMAddress;  }
            if(m_MapAddress){ return m_MapAddress; }
            if(m_SCAddress ){ return m_SCAddress;  }

            return m_EmptyAddress;
        }

    protected:
        int QuerySCAddress();
        int QueryMapAddress();

    protected:
        void DispatchMotion();
};
