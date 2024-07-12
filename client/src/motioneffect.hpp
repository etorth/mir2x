#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
#include "totype.hpp"
#include "sysconst.hpp"

// MotionEffect has a bound MotionNode, assigned to MotionNode::effect
// when MotionNode swapped out by its CO, MotionEffect won't and shouldn't get accessed anymore

// MotionEffect won't support callback
// because MotionNode can be flushed without present frames

// MotionEffect::update() should guarentee that it won't swap motion
// this helps to guarentee MotionEffect::m_motion keeps valid

class MotionNode;
class MotionEffect
{
    protected:
        double m_accuTime;
        MotionNode * const m_motion;

    private:
        const std::pair<const MagicGfxEntry *, const MagicGfxEntryRef *> m_gfxEntryPair;

    protected:
        const MagicGfxEntry    * m_gfxEntry    = m_gfxEntryPair.first;
        const MagicGfxEntryRef * m_gfxEntryRef = m_gfxEntryPair.second;

    public:
        MotionEffect(const char8_t *, const char8_t *, MotionNode *);

    public:
        virtual ~MotionEffect() = default;

    public:
        virtual int speed() const
        {
            return m_gfxEntry->speed;
        }

    public:
        virtual int absFrame() const
        {
            return std::lround((m_accuTime / 1000.0) * SYS_DEFFPS * (to_df(speed()) / 100.0));
        }

        virtual int gfxFrame() const
        {
            return absFrame();
        }

        virtual int frame() const
        {
            return absFrame();
        }

        virtual int frameCount() const
        {
            return m_gfxEntry->frameCount;
        }

    public:
        virtual void drawShift(int, int, uint32_t);

    public:
        virtual void update(double) = 0;

    protected:
        virtual uint32_t frameTexID() const;

    public:
        void reset()
        {
            m_accuTime = 0.0;
        }

    public:
        virtual bool done() const
        {
            return frame() >= frameCount();
        }
};

class Hero;
class HeroSpellMagicEffect: public MotionEffect
{
    protected:
        Hero * const m_hero;

    public:
        HeroSpellMagicEffect(const char8_t *, Hero *, MotionNode *);

    public:
        int frameCount() const override;

    public:
        uint32_t frameTexID() const override;

    public:
        void update(double) override;
};

class ClientCreature;
class MotionAlignedEffect: public MotionEffect
{
    protected:
        ClientCreature * const m_creature;

    private:
        const bool m_useMotionSpeed;

    public:
        MotionAlignedEffect(const char8_t *, const char8_t *, ClientCreature *, MotionNode *, bool useMotionSpeed = true);

    protected:
        int absFrame() const override;

    public:
        void update(double) override;
};

class MotionSyncEffect: public MotionEffect
{
    protected:
        ClientCreature * const m_creature;

    protected:
        const int m_lagFrame;

    public:
        MotionSyncEffect(const char8_t *, const char8_t *, ClientCreature *, MotionNode *, int lagFrame = 0);

    protected:
        int speed() const override;

    protected:
        int gfxFrame() const override;

    public:
        int frameCount() const override;

    public:
        void update(double) override;
};
