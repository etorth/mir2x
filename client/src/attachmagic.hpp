/*
 * =====================================================================================
 *
 *       Filename: attachmagic.hpp
 *        Created: 08/10/2017 12:17:50
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
#include <string_view>
#include "fflerror.hpp"
#include "magicbase.hpp"
#include "magicrecord.hpp"

class ProcessRun;
class AttachMagic: public MagicBase
{
    public:
        AttachMagic(const char8_t *magicName, const char8_t *magicStage, int gfxDirIndex = 0)
            : MagicBase(magicName, magicStage, gfxDirIndex)
        {
            if(!m_gfxEntry.checkType(u8"附着")){
                throw fflerror("invalid magic type: %s", to_cstr(m_gfxEntry.type));
            }
        }

    public:
        virtual void drawShift(int, int, bool) const;
};

class Thunderbolt: public AttachMagic
{
    public:
        Thunderbolt()
            : AttachMagic(u8"雷电术", u8"运行")
        {}

    public:
        void drawShift(int, int, bool) const override;
};
