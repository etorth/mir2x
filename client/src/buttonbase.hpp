#pragma once
#include <cstdint>
#include <variant>
#include <functional>
#include "bevent.hpp"
#include "sysconst.hpp"
#include "widget.hpp"
#include "button.hpp"

class ButtonBase: public Widget
{
    private:
        struct InitArgs
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt w = 0;
            Widget::VarSizeOpt h = 0;

            Button::OverCBFunc onOverIn  = nullptr;
            Button::OverCBFunc onOverOut = nullptr;

            Button::ClickCBFunc onClick = nullptr;
            Button::TriggerCBFunc onTrigger = nullptr;

            Button::SeffIDList seff {};

            int offXOnOver = 0;
            int offYOnOver = 0;

            int offXOnClick = 0;
            int offYOnClick = 0;

            bool onClickDone = true;
            bool radioMode   = false;

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        class InnButtonState final
        {
            private:
                int m_state[2]
                {
                    BEVENT_OFF,
                        BEVENT_OFF,
                };

            public:
                void setState(int state) noexcept
                {
                    m_state[0] = m_state[1];
                    m_state[1] = state;
                }

            public:
                int getState()     const noexcept { return m_state[1]; }
                int getPrevState() const noexcept { return m_state[0]; }
        };

    private:
        InnButtonState m_state;

    protected:
        const bool m_onClickDone;
        const bool m_radioMode;

    protected:
        const Button::SeffIDList m_seff;

    protected:
        const int m_offset[3][2];

    protected:
        Button::OverCBFunc m_onOverIn;
        Button::OverCBFunc m_onOverOut;

    protected:
        Button::ClickCBFunc m_onClick;
        Button::TriggerCBFunc m_onTrigger;

    public:
        ButtonBase(ButtonBase::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    protected:
        int offX() const { return m_offset[getState()][0]; }
        int offY() const { return m_offset[getState()][1]; }

    public:
        int     getState() const { return m_state.    getState(); }
        int getPrevState() const { return m_state.getPrevState(); }

    public:
        void setState(int state)
        {
            m_state.setState(state);
        }

    public:
        bool getOnClickDone() const
        {
            return m_onClickDone;
        }

        bool getRadioMode() const
        {
            return m_radioMode;
        }

    public:
        void setOff () { setState(BEVENT_OFF ); }
        void setOn  () { setState(BEVENT_ON  ); }
        void setDown() { setState(BEVENT_DOWN); }

    private:
        void onOverIn();
        void onOverOut();
        void onClick(bool, int);
        void onTrigger(int);
        void onBadEvent();
};
