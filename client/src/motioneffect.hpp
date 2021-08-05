#pragma once
#include <cstdint>
#include "totype.hpp"
#include "sysconst.hpp"
#include "dbcomrecord.hpp"

class MotionNode;
class MotionEffect
{
    protected:
        double m_accuTime;
        const MagicGfxEntry * const m_gfxEntry;

    protected:
        MotionNode * const m_motion;

    public:
        MotionEffect(const char8_t *, const char8_t *, MotionNode *);

    public:
        virtual int speed() const
        {
            return m_gfxEntry->speed;
        }

    public:
        virtual int absFrame() const
        {
            return (m_accuTime / 1000.0) * SYS_DEFFPS * (to_df(speed()) / 100.0);
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

    protected:
        uint32_t frameTexID() const override;

    public:
        void update(double) override;
};

class ClientCreature;
class MotionSyncEffect: public MotionEffect
{
    protected:
        ClientCreature * const m_creature = nullptr;

    private:
        const bool m_useMotionSpeed = true;

    public:
        MotionSyncEffect(const char8_t *, const char8_t *, ClientCreature *, MotionNode *);

    protected:
        int absFrame() const override;

    public:
        void update(double) override;
};
