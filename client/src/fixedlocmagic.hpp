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
        {}

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
        void drawViewOff(int, int, bool);
};
