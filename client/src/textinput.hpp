#pragma once
#include <functional>
#include "widget.hpp"
#include "inputline.hpp"
#include "labelboard.hpp"
#include "texinputbackground.hpp"

class TextInput: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            const char8_t *labelFirst  = nullptr;
            const char8_t *labelSecond = nullptr;

            Widget::FontConfig font {};

            Widget::VarSize gapFirst  = 3;
            Widget::VarSize gapSecond = 3;

            Widget::VarBool   enableIME {};
            Widget::VarSize2D inputSize {};

            std::function<void()> onTab = nullptr;
            std::function<void()> onCR  = nullptr;

            Widget::WADPair parent {};
        };

    private:
        LabelBoard *m_labelFirst;

    private:
        TexInputBackground m_bg;

    private:
        LabelBoard *m_labelSecond;

    private:
        InputLine m_input;

    public:
        TextInput(TextInput::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m) override
        {
            return m_input.processEventParent(event, valid, m);
        }
};
