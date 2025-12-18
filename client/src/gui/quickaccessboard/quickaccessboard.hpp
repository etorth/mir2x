#pragma once
#include <cstdint>
#include <tuple>
#include "widget.hpp"
#include "tritexbutton.hpp"
#include "gfxshapeboard.hpp"

class ProcessRun;
class QuickAccessBoard: public Widget
{
    private:
        constexpr static uint32_t m_texID = 0X00000060;

    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_bg;
        TritexButton m_buttonClose;

    public:
        QuickAccessBoard(dir8_t,
                int,
                int,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        Widget::ROI getGridLoc(int slot)
        {
            fflassert(slot >= 0, slot);
            fflassert(slot <  6, slot);

            return {17 + 42 * slot, 6, 36, 36};
        }

    public:
        void gridConsume(int);
};
