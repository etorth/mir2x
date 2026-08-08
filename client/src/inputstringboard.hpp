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
    public:
        struct WaitInputArgs final
        {
            std::u8string layoutString {};
            bool security = false;

            std::function<void(std::u8string)> onAccept = nullptr;
            std::function<void()> onReject = nullptr;
            std::function<bool(const std::string &, const std::string &)> validate = nullptr;
        };

    private:
        std::function<void(std::u8string)> m_onAccept;
        std::function<void()> m_onReject;

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
        void acceptInput();

    public:
        void clear()
        {
            m_input.clear();
        }

    public:
        void waitInput(InputStringBoard::WaitInputArgs);

        void waitChoice(
                std::u8string,
                std::function<void()>,
                std::function<void()> = {});
};
