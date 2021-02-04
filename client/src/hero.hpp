/*
 * =====================================================================================
 *
 *       Filename: hero.hpp
 *        Created: 09/03/2015 03:48:41
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
#include <tuple>
#include "creaturemovable.hpp"

class Hero: public CreatureMovable
{
    protected:
        const uint32_t m_DBID;

    protected:
        bool     m_gender;
        uint8_t  m_horse;
        uint16_t m_weapon;

        uint8_t  m_hair;
        uint32_t m_hairColor;

        uint32_t m_dress;
        uint32_t m_dressColor;

    protected:
        bool m_onHorse;

    public:
        Hero(uint64_t, uint32_t, bool, uint32_t, ProcessRun *, const ActionNode &);

    public:
       ~Hero() = default;

    public:
       std::tuple<int, int> location() const override;

    public:
        bool update(double) override;
        void draw(int, int, int) override;

    public:
        bool OnHorse() const
        {
            return m_onHorse;
        }

    public:
        bool motionValid(const std::unique_ptr<MotionNode> &) const override;

    public:
        bool parseAction(const ActionNode &) override;

    public:
        bool     Gender() const { return m_gender ; }
        uint8_t  Horse () const { return m_horse  ; }
        uint16_t Weapon() const { return m_weapon ; }
        uint32_t DBID  () const { return m_DBID   ; }
        uint32_t Dress () const { return m_dress  ; }

    public:
        void Dress(uint32_t nDress)
        {
            m_dress = nDress;
        }

        void Weapon(uint16_t nWeapon)
        {
            m_weapon = nWeapon;
        }

    public:
        bool moving();

    public:
        int motionFrameCount(int, int) const override;

    public:
        int WeaponOrder(int, int, int);

    protected:
        std::unique_ptr<MotionNode> makeWalkMotion(int, int, int, int, int) const override;

    protected:
        int gfxMotionID(int motion) const override
        {
            if((motion >= MOTION_BEGIN) && (motion < MOTION_END)){
                return motion - MOTION_BEGIN;
            }
            return -1;
        }

    protected:
        int GfxDressID (int, int, int) const;
        int GfxWeaponID(int, int, int) const;

    public:
        int maxStep() const override
        {
            return OnHorse() ? 3 : 2;
        }

    public:
        int currStep() const override;

    public:
        virtual void pickUp()
        {
            // need to move this to myhero
        }

    public:
        ClientCreature::TargetBox getTargetBox() const override;
};
