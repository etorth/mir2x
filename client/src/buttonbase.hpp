// basic button class to handle event logic only
// 1. no draw
// 2. no texture id field
//
// I support two callbacks only: off->on and on->click
// this class ask user to configure whether the on->click is triggered
// at the PRESS or RELEASE event.

#pragma once
#include <cstdint>
#include <functional>

#include "widget.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"

class ButtonBase: public Widget
{
    private:
        class InnButtonState final
        {
            // encapsulate it as a class
            // don't let button class to manipulate m_state directly
            private:
                int m_state[2]
                {
                    BEVENT_OFF,
                    BEVENT_OFF,
                };

            public:
                void setState(int state)
                {
                    m_state[0] = m_state[1];
                    m_state[1] = state;
                }

            public:
                int getState() const
                {
                    return m_state[1];
                }

                int getLastState() const
                {
                    return m_state[0];
                }
        };

    private:
        InnButtonState m_state;

    protected:
        const bool m_onClickDone;
        const bool m_radioMode;

    protected:
        const std::optional<uint32_t> m_seffID[3];

    protected:
        const int m_offset[3][2];

    protected:
        std::function<void(Widget *)> m_onOverIn;
        std::function<void(Widget *)> m_onOverOut;
        std::function<void(Widget *)> m_onClick;

    public:
        ButtonBase(Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,
                Widget::VarSize,
                Widget::VarSize,

                std::function<void(Widget *)> = nullptr,
                std::function<void(Widget *)> = nullptr,
                std::function<void(Widget *)> = nullptr,

                std::optional<uint32_t> = {},
                std::optional<uint32_t> = {},
                std::optional<uint32_t> = {},

                int = 0,
                int = 0,
                int = 0,
                int = 0,

                bool = true,
                bool = false,

                Widget * = nullptr,
                bool     = false);

    public:
        virtual ~ButtonBase() = default;

    public:
        bool processEventDefault(const SDL_Event &, bool) override;

    protected:
        int offX() const
        {
            return m_offset[getState()][0];
        }

        int offY() const
        {
            return m_offset[getState()][1];
        }

    public:
        int getState() const
        {
            return m_state.getState();
        }

        int getLastState() const
        {
            return m_state.getLastState();
        }

    public:
        void setState(int state)
        {
            m_state.setState(state);
        }

    public:
        void setOff () { setState(BEVENT_OFF ); }
        void setOn  () { setState(BEVENT_ON  ); }
        void setDown() { setState(BEVENT_DOWN); }

    private:
        void onClick();
        void onOverIn();
        void onOverOut();
        void onBadEvent();
};
