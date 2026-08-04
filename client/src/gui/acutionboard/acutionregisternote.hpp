#pragma once
#include <string>
#include "widget.hpp"
#include "textboard.hpp"
#include "layoutboard.hpp"

class AcutionRegisterNote final: public Widget
{
    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarInt enableIME = IME_DISABLE;

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        bool m_enabled = true;

    private:
        LayoutBoard m_note;
        TextBoard m_placeholder;

    public:
        explicit AcutionRegisterNote(AcutionRegisterNote::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setInputEnabled(bool enabled)
        {
            m_enabled = enabled;
        }

        void setInputFocus(bool focused)
        {
            m_note.setFocus(focused);
        }

    public:
        std::string getText() const
        {
            return m_note.getText();
        }

        void clear()
        {
            m_note.clear();
        }
};
