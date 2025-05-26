#pragma once
#include <cmath>
#include <string_view>
#include "totype.hpp"
#include "fflerror.hpp"
#include "basemagic.hpp"
#include "magicrecord.hpp"

class AttachMagic: public BaseMagic
{
    public:
        AttachMagic(const char8_t *magicName, const char8_t *magicStage, int gfxDirIndex = 0)
            : BaseMagic(magicName, magicStage, gfxDirIndex)
        {
            fflassert(m_gfxEntry->checkType(u8"附着"));
        }

    public:
        virtual void drawShift(int, int, uint32_t) const;
};

class Thunderbolt: public AttachMagic
{
    public:
        Thunderbolt()
            : Thunderbolt(u8"雷电术")
        {}

        Thunderbolt(const char8_t *magicName)
            : AttachMagic(magicName, u8"运行")
        {
            fflassert(m_gfxEntryRef);
            fflassert(to_u8sv(m_gfxEntryRef->name) == u8"魔法特效_雷电术", m_gfxEntryRef->name);
        }

    public:
        void drawShift(int, int, uint32_t) const override;
};

class TaoYellowBlueRing: public AttachMagic
{
    public:
        TaoYellowBlueRing()
            : AttachMagic(u8"阴阳法环", u8"运行")
        {}

    public:
        void drawShift(int, int, uint32_t) const;
};

class AntHealing: public AttachMagic
{
    public:
        AntHealing()
            : AttachMagic(u8"蚂蚁道士_治疗", u8"运行")
        {}

    public:
        void drawShift(int, int, uint32_t) const;
};
