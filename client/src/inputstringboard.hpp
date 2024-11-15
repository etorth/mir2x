#pragma once
#include <vector>
#include <cstdint>
#include "widget.hpp"
#include "passwordbox.hpp"
#include "tritexbutton.hpp"

class InputStringBoard: public Widget
{
    private:
        PasswordBox m_input;

    private:
        TritexButton m_yesButton;
        TritexButton m_nopButton;

    private:
        std::u8string m_parXMLString;
        std::function<void(std::u8string)> m_onDone;

    public:
        InputStringBoard(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                bool,

                Widget * = nullptr,
                bool     = false);

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool) override;

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
