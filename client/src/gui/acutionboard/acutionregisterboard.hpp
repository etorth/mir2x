#pragma once
#include <cstdint>
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "imageboard.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class AcutionRegisterBoard final: public Widget
{
    private:
        constexpr static int m_noteX = 9;
        constexpr static int m_noteY = 68;
        constexpr static int m_noteW = 222;
        constexpr static int m_noteH = 116;

        constexpr static int m_itemX = 241;
        constexpr static int m_itemY = 45;
        constexpr static int m_itemW = 159;
        constexpr static int m_itemH = 164;

        constexpr static int m_priceX = 25;
        constexpr static int m_priceY = 232;
        constexpr static int m_priceW = 145;
        constexpr static int m_priceH = 27;

    private:
        ProcessRun *m_runProc;

    private:
        SDItem m_item;
        uint64_t m_price = 0;
        bool m_pending = false;
        bool m_dragging = false;

    private:
        ImageBoard m_background;
        Widget m_noteArea;
        LayoutBoard m_noteBoard;
        TritexButton m_buttonRegister;
        TritexButton m_buttonCancel;
        TritexButton m_buttonClose;

    public:
        AcutionRegisterBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void begin();

    public:
        void drawDefault(Widget::ROIMap) const override;
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    private:
        void setPending(bool);
        void setPrice();
        void confirmRegister();
        void registerItem();
        void confirmCancel();

    private:
        void restoreGrabbedItem();
        void restoreOwnedItem();
        void closeRegister(bool);
};
