#pragma once
#include "widget.hpp"
#include "texslider.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class QuestStateBoard: public Widget
{
    private:
        bool m_left = true;

    private:
        TexSlider m_slider;

    private:
        TritexButton m_lrButton;
        TritexButton m_closeButton;

    private:
        ProcessRun *m_processRun;

    public:
        QuestStateBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;
};
