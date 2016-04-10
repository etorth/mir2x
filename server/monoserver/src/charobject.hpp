#pragma once

#include <vector>
#include <forward_list>

#include "monoserver.hpp"

enum ObjectType: uint8_t{
    OT_HUMAN    = 0,
    OT_PLAYER,
    OT_NPC,
    OT_ANIMAL,
    OT_MONSTER,
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

class CharObject
{
    public:
        CharObject();
        ~CharObject();

    public:
        virtual bool Friend(const CharObject *) const = 0;
        virtual void Operate() = 0;

    public:
        uint8_t GetBack()
        {
            switch (m_Direction){
                case DR_UP       : return DR_DOWN;
                case DR_DOWN     : return DR_UP;
                case DR_LEFT     : return DR_RIGHT;
                case DR_RIGHT    : return DR_LEFT;
                case DR_UPLEFT   : return DR_DOWNRIGHT;
                case DR_UPRIGHT  : return DR_DOWNLEFT;
                case DR_DOWNLEFT : return DR_UPRIGHT;
                case DR_DOWNRIGHT: return DR_UPLEFT;
                default: break;
            }

            // make the compiler happy
            return m_Direction;
        }

    protected:
        uint16_t m_ObjectType;

    public:
        virtual bool Type(uint8_t) = 0;

    protected:
        int     m_CurrX;
        int     m_CurrY;
        int     m_Direction;
        int     m_Event;

        PLAYERFEATURE               m_Feature;
        PLAYERFEATUREEX             m_FeatureEx;
        ServerMap*                  m_Map;

        std::forward_list<std::tuple<uint32_t, uint32_t>> m_VisibleObjectList;
        std::forward_list<std::tuple<uint32_t, uint32_t>> m_VisibleItemList;
        std::forward_list<std::tuple<uint32_t, uint32_t>> m_VisibleEventList;
        std::forward_list<std::tuple<uint32_t, uint32_t>> m_CacheObjectList;

        OBJECTABILITY               m_Ability;
        OBJECTABILITY               m_WAbility;
        OBJECTADDABILITY            m_AddAbility;

        std::array<uint8_t,  256>    m_StateAttrV;
        std::array<uint32_t, 256>    m_StateTimeV;

        std::tuple<uint32_t, uint32_t> m_Master;
        std::tuple<uint32_t, uint32_t> m_Target;

    public:

        bool    GetNextPosition(int nSX, int nSY, int nDir, int nDistance, int& nX, int& nY);
        int     GetNextDirection(int nStartX, int nStartY, int nTargetX, int nTargetY);
        void    TurnTo(int nDir);
        bool    TurnXY(int nX, int nY, int nDir);
        virtual bool    WalkTo(int nDir);
        bool    WalkXY(int nX, int nY, int nDir);
        bool    RunTo(int nDir);
        bool    RunXY(int nX, int nY, int nDir);
        bool    HitXY(uint16_t wIdent, int nX, int nY, int nDir, int nHitStyle);

        void    Disappear();
        void    MakeGhost();

        bool    DropItem(_LPTUSERITEMRCD lpTItemRcd, int nRange, bool fIsGenItem);

        CharObject* AddCreatureSysop(int nX, int nY, CMonRaceInfo* pMonRaceInfo, bool fSearch);


        void    SpaceMove(int nX, int nY, CMirMap* pMirMap);
        void    Die();
        virtual int   NameColorType() = 0;
        virtual const char *CharName()  = 0;
        virtual void    Operate() = 0;
        virtual void    SearchViewRange() = 0;

    public:
        // it's locked for sure, then all its related variables stay valid
        void Unlock()
        {
            if(m_Lock){
                m_Lock->lock();
            }
        }

    public:
        bool Target(uint32_t nID, uint32_t nAddTime)
        {
            extern MonoServer *g_MonoServer;
            if(g_MonoServer->CheckOut<CharObject, false>(nID, nAddTime)){
                m_Target = {nID, nAddTime};
                return true;
            }
            return false;
        }
};
