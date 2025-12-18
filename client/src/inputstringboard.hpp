#pragma once
#include <vector>
#include <cstdint>
#include "widget.hpp"
#include "imageboard.hpp"
#include "passwordbox.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"
#include "gfxshapeboard.hpp"

class InputStringBoard: public Widget
{
    private:
        std::function<void(std::u8string)> m_onDone;

    private:
        ImageBoard m_bg;

    private:
        LayoutBoard m_textInfo;

    private:
        GfxShapeBoard m_inputBg;
        PasswordBox    m_input;

    private:
        TritexButton m_yesButton;
        TritexButton m_nopButton;

    public:
        InputStringBoard(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                bool,

                Widget * = nullptr,
                bool     = false);

    private:
        void inputLineDone();

    public:
        void clear()
        {
            m_input.clear();
        }

    public:
        void waitInput(std::u8string, std::function<void(std::u8string)>);

    public:
        void setSecurity(bool security)
        {
            m_input.setSecurity(security);
        }
};
