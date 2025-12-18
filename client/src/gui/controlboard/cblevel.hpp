#pragma once
#include <functional>
#include "widget.hpp"
#include "bevent.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"
#include "buttonbase.hpp"

class ProcessRun;
class CBLevel: public ButtonBase
{
    protected:
        using ButtonBase::TriggerCBFunc;

    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_image;
        TextBoard  m_level;

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
