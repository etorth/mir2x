#pragma once
#include <cstdint>
#include <string>
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"
#include "stallitemgrid.hpp"

// stall board works in 3 modes:
//
//   1. seller setup  : items moved from inventory, can be removed, 0X910 starts selling
//   2. seller selling: items can be removed with confirmation, 0X860 closes the stall
//   3. buyer         : selected item can be bought by 0XB7
//
// only one op button shows in the op slot at any time

class ProcessRun;
class StallBoard final: public Widget
{
    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            ProcessRun *runProc = nullptr;
            Widget::WADPair parent {};
        };

    private:
        ProcessRun *m_runProc;

    private:
        bool m_dragging = false;

    private:
        SDStall m_stall;

    private:
        ImageBoard m_background;

    private:
        TextBoard m_sellerName;
        TextBoard m_stallName;

    private:
        StallItemGrid m_grid;

    private:
        TextBoard m_infoLine0;
        TextBoard m_infoLine1;

    private:
        ImageBoard   m_opBackground;
        TritexButton m_buttonItemOp;
        TritexButton m_buttonOpenStall;
        TritexButton m_buttonCloseStall;

    private:
        TritexButton m_buttonClose;

    public:
        explicit StallBoard(StallBoard::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void loadStall(SDStall);

        uint32_t stallDBID() const
        {
            return m_stall.dbid;
        }

    private:
        bool isSeller() const;

    private:
        bool showItemOp() const;
        bool showOpenStall() const;
        bool showCloseStall() const;

    private:
        void onItemOp();
        void onOpenStall();
        void onCloseStall();
        void onClose();

    private:
        void removeItem(const SDItem &);
        void buyItem(const SDItem &, size_t, size_t);

    private:
        void drawItemHoverText(const SDItem &, size_t) const;
};
