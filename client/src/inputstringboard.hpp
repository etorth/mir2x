#pragma once
#include <vector>
#include <cstdint>
#include "widget.hpp"
#include "imageboard.hpp"
#include "inputline.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"
#include "gfxshapeboard.hpp"
#include "radioselector.hpp"

class InputStringBoard: public Widget
{
    public:
        struct WaitInputArgs final
        {
            std::u8string layoutString {};
            bool security = false;
            bool modal    = false;

            std::function<void(std::u8string)>            onAccept = nullptr;
            std::function<void()>                         onReject = nullptr;
            std::function<bool(std::string, std::string)> validate = nullptr;
        };

    private:
        std::function<void(std::u8string)> m_onAccept;
        std::function<void()> m_onReject;

    private:
        bool m_modal = false;

    private:
        ImageBoard m_bg;

    private:
        LayoutBoard m_textInfo;

    private:
        GfxShapeBoard m_inputBg;
        InputLine m_input;

    private:
        RadioSelector *m_choice = nullptr;

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

    private:
        void   resetChoice();
        size_t selectedChoice() const;

    private:
        int textInfoH() const;
        int choiceGap() const;
        int contentY () const;

    public:
        void clear()
        {
            m_input.clear();
        }

    public:
        bool showModal() const
        {
            return show() && m_modal;
        }

    public:
        void waitInput(InputStringBoard::WaitInputArgs);
        void waitChoice(std::u8string, std::vector<std::u8string>, bool, std::function<void(size_t)>, std::function<void()> = {});
};
