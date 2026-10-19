#pragma once
#include <optional>
#include "widget.hpp"
#include "imageboard.hpp"
#include "gfxresizeboard.hpp"
#include "textboard.hpp"
#include "serdesmsg.hpp"

class ProcessRun;
class ClientCreature;

class HPBarBoard: public Widget
{
    private:
        struct InitArgs
        {
            ProcessRun *runProc = nullptr;
            ClientCreature *creature = nullptr;
        };

    public:
        static constexpr int BAR_W_MIN    = 18;
        static constexpr int BAR_W_MARGIN =  3;

    private:
        ProcessRun *m_runProc;
        ClientCreature *m_creature;

    private:
        Widget m_bar;
        Widget m_fillClip;

    private:
        ImageBoard m_fill;
        ImageBoard m_frame;
        GfxResizeBoard m_resizedFrame;

    private:
        TextBoard m_numbers;
        Widget    m_iconArea;

    private:
        bool showNumber() const;
        bool showBar   () const;

    private:
        int     barWidthClip() const;
        int baseBarWidth    () const;
        int     barWidth    () const;
        int     barHeight   () const;

    private:
        int buffIconsPerRow() const;

    public:
        HPBarBoard(HPBarBoard::InitArgs);
        void setBuffIDList(const SDBuffIDList &);
};
