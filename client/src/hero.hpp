/*
 * =====================================================================================
 *
 *       Filename: hero.hpp
 *        Created: 09/03/2015 03:48:41
 *  Last Modified: 06/26/2017 00:05:29
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
#include "creature.hpp"

class Hero: public Creature
{
    protected:
        const uint32_t m_DBID;

    protected:
        bool     m_Gender;
        uint8_t  m_Horse;
        uint16_t m_Weapon;

        uint8_t  m_Hair;
        uint32_t m_HairColor;

        uint8_t  m_Dress;
        uint32_t m_DressColor;

    protected:
        bool m_OnHorse;

    public:
        Hero(uint32_t, uint32_t, bool, uint32_t, ProcessRun *, const ActionNode &);

    public:
       ~Hero() = default;

    public:
        bool Location(int *, int *);

    public:
        bool Draw(int, int);
        bool Update();

    public:
        bool CanFocus(int, int);

    public:
        bool ActionValid(const ActionNode &);
        bool MotionValid(const MotionNode &);

    public:
        virtual int32_t GfxID(int, int, int);

    public:
        bool UpdateMotion();

    public:
        bool ParseNewAction(const ActionNode &, bool);

    public:
        int Type()
        {
            return CREATURE_PLAYER;
        }

    public:
        bool     Gender() const { return m_Gender ; }
        uint8_t  Horse () const { return m_Horse  ; }
        uint16_t Weapon() const { return m_Weapon ; }
        uint32_t DBID  () const { return m_DBID   ; }
        uint32_t Dress () const { return m_Dress  ; }

    public:
        void Dress(uint32_t nDress)
        {
            m_Dress = nDress;
        }

        void Weapon(uint16_t nWeapon)
        {
            m_Weapon = nWeapon;
        }

    public:
        bool Moving();

    public:
        size_t MotionFrameCount();

    public:
        bool WeaponOrder(int, int, int);

    public:
        bool ValidG()
        {
            return true;
        }
};
