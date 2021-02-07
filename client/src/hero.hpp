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
#include <array>
#include "creaturemovable.hpp"

class Hero: public CreatureMovable
{
    protected:
        uint8_t m_horse;
        bool    m_onHorse;

    protected:
        std::string m_name;
        uint32_t    m_nameColor;

    protected:
        PlayerLook m_look;
        PlayerWear m_wear;

    public:
        Hero(uint64_t, const PlayerLook &, ProcessRun *, const ActionNode &);

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
        uint8_t horse() const
        {
            return m_horse;
        }

    public:
        const PlayerLook & getPlayerLook() const { return m_look; }
        const PlayerWear & getPlayerWear() const { return m_wear; }

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
        int GfxHairID  (int, int, int) const;
        int GfxDressID (int, int, int) const;
        int GfxWeaponID(int, int, int) const;
        int gfxHelmetID(int, int, int) const;

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

    public:
        void setWear(const PlayerWear &wear)
        {
            m_wear = wear;
        }

        void setLook(const PlayerLook &look)
        {
            m_look = look;
        }

        void setName(const char *name, uint32_t nameColor)
        {
            m_name = to_cstr(name);
            m_nameColor = nameColor;
        }

    public:
        std::string getName() const
        {
            return m_name;
        }

        uint32_t getNameColor() const
        {
            return m_nameColor;
        }

    public:
        uint32_t getWLGridItemID(int);
        bool     setWLGridItemID(int, uint32_t);
};
