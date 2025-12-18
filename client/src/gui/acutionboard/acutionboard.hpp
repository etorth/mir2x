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
};
