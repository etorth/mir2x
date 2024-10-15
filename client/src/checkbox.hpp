#pragma once
#include <cstdint>
#include "widget.hpp"
#include "sdldevice.hpp"
#include "imageboard.hpp"

class CheckBox: public Widget
{
    private:
        uint32_t m_color;

    private:
        bool m_innVal = false;

    private:
        std::function<bool(const Widget *      )> m_valGetter;
        std::function<void(      Widget *, bool)> m_valSetter;
        std::function<void(      Widget *, bool)> m_valOnChange;

    private:
        ImageBoard m_checkImage;

    public:
        CheckBox(
                dir8_t,
                int,
                int,
                int,
                int,

                uint32_t,

                std::function<bool(const Widget *      )>, // getter, use m_innVal if not provided
                std::function<void(      Widget *, bool)>, // setter
                std::function<void(      Widget *, bool)>, // onchange

                Widget * = nullptr, // parent
                bool     = false);  // auto-delete

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void setColor(uint32_t color)
        {
            m_color = color;
        }

    public:
        void toggle();

    public:
        bool getter(    ) const;
        void setter(bool);

    private:
        static SDL_Texture *loadFunc(const ImageBoard *);
};
