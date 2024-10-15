#pragma once
#include "widget.hpp"
#include "sdldevice.hpp"
#include "checkbox.hpp"
#include "labelboard.hpp"

class CheckLabel: public Widget
{
    private:
        const uint32_t m_checkBoxColor;
        const uint32_t m_labelBoardColor;

    private:
        CheckBox m_checkBox;
        LabelBoard m_labelBoard;

    public:
        CheckLabel(
                dir8_t,
                int,
                int,

                bool,
                int,

                uint32_t,
                int,
                int,

                std::function<bool(const Widget *      )>,
                std::function<void(      Widget *, bool)>,
                std::function<void(      Widget *, bool)>,

                const char8_t *,
                uint8_t,
                uint8_t,
                uint8_t,
                uint32_t,

                Widget * = nullptr, // parent
                bool     = false);  // auto-delete

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void setFocus(bool) override;

    public:
        bool getter() const
        {
            return m_checkBox.getter();
        }

        void setter(bool val)
        {
            m_checkBox.setter(val);
        }
};
