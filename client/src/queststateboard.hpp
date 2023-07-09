#pragma once
#include "widget.hpp"
#include "sysconst.hpp"
#include "labelboard.hpp"
#include "protocoldef.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class QuestStateBoard: public Widget
{
    private:
        TritexButton m_closeButton;

    private:
        ProcessRun *m_processRun;

    public:
        PlayerStateBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;
        void drawWear();

    public:
        bool processEvent(const SDL_Event &, bool) override;

    private:
        void drawItemHoverText(int) const;
};
