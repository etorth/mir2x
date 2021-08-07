#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
#include "totype.hpp"
#include "sysconst.hpp"
#include "dbcomrecord.hpp"

// MotionEffect has a bound MotionNode, assigned to MotionNode::effect
// when MotionNode swapped out by its CO, MotionEffect won't and shouldn't get accessed anymore

// MotionEffect won't support callback
// because MotionNode can be flushed without present frames

class MotionNode;
class MotionEffect
{
    protected:
        double m_accuTime;
        MotionNode * const m_motion;

    protected:
        const MagicGfxEntry * const m_gfxEntry;

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

    public:
        MotionSyncEffect(const char8_t *, const char8_t *, ClientCreature *, MotionNode *);

    protected:
        int absFrame() const override;

    public:
        void update(double) override;
};
