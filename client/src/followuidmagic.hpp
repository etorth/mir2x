/*
 * =====================================================================================
 *
 *       Filename: followuidmagic.hpp
 *        Created: 12/07/2020 21:19:44
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
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "magicbase.hpp"

class ProcessRun;
class FollowUIDMagic: public MagicBase
{
    protected:
        int m_x;
        int m_y;
        int m_moveSpeed;

        uint64_t m_uid;
        ProcessRun *m_process;

    public:
        FollowUIDMagic(const char8_t *, const char8_t *, int, int, int, int, uint64_t, ProcessRun *);

    public:
        bool done() const override;

    public:
        void update(double) override;

    public:
        virtual void drawViewOff(int, int, bool);
};

class TaoFireFigure_RUN: public FollowUIDMagic
{
    public:
        TaoFireFigure_RUN(int x, int y, int gfxDirIndex, uint64_t aimUID, ProcessRun *runPtr)
            : FollowUIDMagic(u8"灵魂火符", u8"运行", x, y, gfxDirIndex, 25, aimUID, runPtr)
        {}

    public:
        void drawViewOff(int, int, bool) override;
};
