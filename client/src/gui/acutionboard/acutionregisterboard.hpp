#pragma once
#include "widget.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class AcutionRegisterBoard final: public Widget
{
    private:
        ProcessRun *m_runProc;

    private:
        ImageBoard m_background;
        TritexButton m_buttonRegister;
        TritexButton m_buttonCancel;
        TritexButton m_buttonClose;

    public:
        AcutionRegisterBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;
};
