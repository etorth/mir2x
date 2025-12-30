#pragma once
#include "widget.hpp"
#include "trigfxbutton.hpp"

class ProcessRun;
class CBLevel: public TrigfxButton
{
    protected:
        using TrigfxButton::TriggerCBFunc;

    private:
        ProcessRun *m_processRun;

    private:
        Widget m_canvas;

    public:
        CBLevel(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                ProcessRun *,
                CBLevel::TriggerCBFunc,

                Widget * = nullptr,
                bool     = false);
};
