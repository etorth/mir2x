#pragma once
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "texslider.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class HorseBoard: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        TritexButton m_close;

    private:
        TritexButton m_up;
        TritexButton m_down;
        TritexButton m_hide;
        TritexButton m_show;

    public:
        HorseBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;
};
