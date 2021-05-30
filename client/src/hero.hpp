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
#include "serdesmsg.hpp"
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
        SDWLDesp m_sdWLDesp;

    public:
        Hero(uint64_t, ProcessRun *, const ActionNode &);

    public:
       ~Hero() = default;

    public:
        bool update(double) override;

    public:
        void drawFrame(int, int, int, int, bool) override;

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

        bool gender() const
        {
            return uidf::getPlayerGender(UID());
        }

    public:
        bool moving();

    public:
        FrameSeq motionFrameSeq(int, int) const override;

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
        ClientCreature::TargetBox getTargetBox() const override;

    public:
        void setWLDesp(SDWLDesp desp)
        {
            m_sdWLDesp = std::move(desp);
        }

        void setName(const char *name, uint32_t nameColor)
        {
            m_name = to_cstr(name);
            m_nameColor = (colorf::RGBMask(nameColor) ? nameColor : colorf::WHITE) | 0XFF;
        }

        const auto &getWLDesp() const
        {
            return m_sdWLDesp;
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
        const SDItem &getWLItem(int) const;
        bool          setWLItem(int, SDItem);

    public:
        void jumpLoc(int, int, int);

    protected:
        std::unique_ptr<MotionNode> makeIdleMotion() const override
        {
            return std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_STAND,
                .direction = m_currMotion->direction,
                .x = m_currMotion->endX,
                .y = m_currMotion->endY,
            });
        }

    protected:
        bool deadFadeOut() override;

    public:
        void setBuff(int, int) override;
};
