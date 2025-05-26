#pragma once
#include "widget.hpp"
#include "imageboard.hpp"

class ProcessPrun;
class AcutionBoard: public Widget
{
    private:
        ProcessRun *m_runProc;

    private:
        ImageBoard m_background;

    public:
        AcutionBoard(ProcessRun *, Widget * = nullptr, bool = false);

    // public:
    //     void drawEx(int, int, int, int, int, int) const override;
    //
    // public:
    //     bool processEventDefault(const SDL_Event &, bool) override;
};
