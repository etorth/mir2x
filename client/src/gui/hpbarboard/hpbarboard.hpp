#pragma once
#include <optional>
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "imageboard.hpp"
#include "gfxresizeboard.hpp"
#include "textboard.hpp"
#include "itemflex.hpp"

class ProcessRun;
class ClientCreature;

class HPBarBoard: public Widget
{
    private:
        struct InitArgs
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x   = 0;
            Widget::VarInt y   = 0;

            ProcessRun *runProc = nullptr;
            ClientCreature *creature = nullptr;

            Widget::WADPair  parent {};
        };

    public:
        static constexpr int BAR_W_MIN    = 40;
        static constexpr int BAR_W_MARGIN =  0; // disable inset currently

    private:
        ProcessRun *m_runProc;
        ClientCreature *m_creature;

    private:
        Widget *m_iconArea = nullptr;

    private:
        TextBoard  m_numbers;
        ImageBoard m_empty;
        ImageBoard m_full;

    private:
        GfxResizeBoard m_resizeEmpty;
        GfxResizeBoard m_resizeFull;

    private:
        GfxCropBoard m_resizeFullCrop;

    private:
        ItemFlex m_flex;

    public:
        HPBarBoard(HPBarBoard::InitArgs);

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
        void setBuffIDList(const SDBuffIDList &);
};
