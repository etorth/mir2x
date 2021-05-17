/*
 * =====================================================================================
 *
 *       Filename: fixedlocmagic.hpp
 *        Created: 08/07/2017 21:19:44
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
#include <cstdint>
#include <cstdlib>
#include "totype.hpp"
#include "fflerror.hpp"
#include "magicbase.hpp"
#include "magicrecord.hpp"

class FixedLocMagic: public MagicBase
{
    private:
        int m_x;
        int m_y;

    public:
        FixedLocMagic(
                const char8_t *magicName,
                const char8_t *magicStage,

                int x,
                int y,
                int gfxDirIndex = -1)
            : MagicBase(magicName, magicStage, gfxDirIndex)
            , m_x(x)
            , m_y(y)
        {
            fflassert(m_gfxEntry.checkType(u8"固定"));
        }

    public:
        int x() const
        {
            return m_x;
        }

        int y() const
        {
            return m_y;
        }

    public:
        virtual void drawViewOff(int, int, uint32_t) const;
};

class FireAshEffect_RUN: public FixedLocMagic
{
    private:
        const int m_absFrameOff;

    private:
        const int m_alphaTime[3];

    public:
        FireAshEffect_RUN(int x, int y, int t1 = 2000, int t2 = 5000, int t3 = 3000)
            : FixedLocMagic(u8"火焰灰烬", u8"运行", x, y, std::rand() % 5)
            , m_absFrameOff(std::rand() % 10)
            , m_alphaTime
              {
                  t1,
                  t2,
                  t3,
              }
        {
            fflassert(t1 >  0);
            fflassert(t2 >= 0);
            fflassert(t3 >  0);
        }

    private:
        int absFrame() const override
        {
            return FixedLocMagic::absFrame() + m_absFrameOff;
        }

    private:
        bool done() const override
        {
            return m_accuTime >= m_alphaTime[0] + m_alphaTime[1] + m_alphaTime[2];
        }

    private:
        uint32_t getPlainModColor() const
        {
            return colorf::WHITE + colorf::round255(255.0 * [this]() -> float
            {
                if(m_accuTime < m_alphaTime[0]){
                    return to_f(m_accuTime) / m_alphaTime[0];
                }
                else if(m_accuTime < m_alphaTime[0] + m_alphaTime[1]){
                    return 1.0f;
                }
                else if(m_accuTime < m_alphaTime[0] + m_alphaTime[1] + m_alphaTime[2]){
                    return 1.0f - to_f(m_accuTime - m_alphaTime[0] - m_alphaTime[1]) / m_alphaTime[2];
                }
                else{
                    return 0.0f;
                }
            }());
        }

    protected:
        void drawViewOff(int, int, uint32_t) const override;

    public:
        void drawGroundAsh(int, int, uint32_t) const;
};

class FireWall_RUN: public FixedLocMagic
{
    private:
        const int m_absFrameOff;

    private:
        int m_fadeDuration = 0;
        double m_fadeStartTime = 0.0f;

    public:
        FireWall_RUN(int x, int y)
            : FixedLocMagic(u8"火墙", u8"运行", x, y)
            , m_absFrameOff(std::rand() % 10)
        {}

    private:
        int absFrame() const override
        {
            return FixedLocMagic::absFrame() + m_absFrameOff;
        }

    public:
        void setFadeOut(int fadeDuration)
        {
            fflassert(fadeDuration > 0);
            fflassert(!hasFadeOut());

            m_fadeDuration = fadeDuration;
            m_fadeStartTime = m_accuTime;
        }

        bool hasFadeOut() const
        {
            return m_fadeDuration > 0;
        }

    private:
        bool done() const override
        {
            if(hasFadeOut()){
                return m_accuTime >= m_fadeStartTime + m_fadeDuration;
            }
            else{
                return false;
            }
        }

    protected:
        void drawViewOff(int, int, uint32_t) const override;

    private:
        uint32_t getPlainModColor() const
        {
            if(hasFadeOut()){
                return colorf::WHITE + colorf::round255(255.0f * (1.0f - std::min<float>((m_accuTime - m_fadeStartTime) / to_f(m_fadeDuration), 1.0f)));
            }
            else{
                return colorf::WHITE + 255;
            }
        }
};

class HellFire_RUN: public FixedLocMagic
{
    private:
        FixedLocMagic m_fireRun0;
        FixedLocMagic m_fireRun1;

    private:
        int m_fireDir;

    public:
        HellFire_RUN(int x, int y, int dir)
            : FixedLocMagic(u8"地狱火", u8"运行", x, y)
            , m_fireRun0   (u8"地狱火", u8"运行", x, y, (dir + 0) % 2)
            , m_fireRun1   (u8"地狱火", u8"运行", x, y, (dir + 1) % 2)
            , m_fireDir(dir)
        {
            fflassert(directionValid(m_fireDir));
        }

    protected:
        bool update(double ms) override
        {
            m_fireRun0.update(ms);
            m_fireRun1.update(ms);
            return FixedLocMagic::update(ms);
        }

    protected:
        void drawViewOff(int, int, uint32_t) const override;
};
