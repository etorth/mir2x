#pragma once
#include <tuple>
#include <array>
#include <optional>
#include "serdesmsg.hpp"
#include "creaturemovable.hpp"

struct HeroFrameGfxSeq final
{
    const int count = 0;

    operator bool() const
    {
        return count > 0;
    }
};

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

    protected:
        std::unordered_set<uint32_t> m_swingMagicList;

    public:
        Hero(uint64_t, ProcessRun *, const ActionNode &);

    public:
       ~Hero() = default;

    public:
        bool update(double) override;

    public:
        void drawFrame(int, int, int, int, bool) override;

    public:
        bool onHorse() const
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
        HeroFrameGfxSeq getFrameGfxSeq(int, int) const;

    public:
        int getFrameCount(int motion, int direction) const override
        {
            return getFrameGfxSeq(motion, direction).count;
        }

    public:
        static std::optional<int> weaponOrder(int, int, int);

    protected:
        std::unique_ptr<MotionNode> makeWalkMotion(int, int, int, int, int) const override;

    protected:
        static std::optional<uint32_t> gfxMotionID(int motion)
        {
            if((motion >= MOTION_BEGIN) && (motion < MOTION_END)){
                return motion - MOTION_BEGIN;
            }
            return {};
        }

    protected:
        std::optional<uint32_t> gfxHairID  (int, int, int) const;
        std::optional<uint32_t> gfxDressID (int, int, int) const;
        std::optional<uint32_t> gfxWeaponID(int, int, int) const;
        std::optional<uint32_t> gfxHelmetID(int, int, int) const;

    public:
        int maxStep() const override
        {
            return onHorse() ? 3 : 2;
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
            m_nameColor = (colorf::maskRGB(nameColor) ? nameColor : colorf::WHITE) + colorf::A_SHF(0XFF);
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
        bool          setWLItem(int, SDItem, bool playSound = false);

    public:
        void jumpLoc(int, int, int);

    protected:
        std::unique_ptr<MotionNode> makeIdleMotion() const override
        {
            return std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = onHorse() ? MOTION_ONHORSESTAND : MOTION_STAND,
                .direction = m_currMotion->direction,
                .x = m_currMotion->endX,
                .y = m_currMotion->endY,
            });
        }

    protected:
        bool deadFadeOut() override;

    public:
        void setBuff(int, int) override;

    public:
        bool hasSwingMagic(uint32_t) const;
        void toggleSwingMagic(uint32_t, std::optional<bool> = {});
};
