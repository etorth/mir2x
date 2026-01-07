#pragma once
#include "widget.hpp"
#include "trigfxbutton.hpp"

class ProcessRun;
class CBLevel: public TrigfxButton
{
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
                Button::TriggerCBFunc,

                Widget * = nullptr,
                bool     = false);
};
