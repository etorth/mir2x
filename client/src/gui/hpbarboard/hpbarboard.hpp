#pragma once
#include <optional>
#include "widget.hpp"
#include "imageboard.hpp"
#include "textboard.hpp"
#include "serdesmsg.hpp"

class ProcessRun;

class HPBarBoard: public Widget
{
    private:
        ProcessRun *m_processRun;
        const std::optional<SDHealth> &m_health;
        const double &m_animationTime;

        Widget m_bar;
        Widget m_fillClip;
        ImageBoard m_fill;
        ImageBoard m_frame;
        TextBoard m_numbers;
        Widget m_iconArea;

    private:
        bool numericHealth() const;
        bool showBar() const;
        int barWidth() const;
        int barHeight() const;
        double healthRatio() const;

    public:
        HPBarBoard(ProcessRun *, const std::optional<SDHealth> &, const double &);
        void setBuffIDList(const SDBuffIDList &);
};
