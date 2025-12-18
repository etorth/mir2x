#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <SDL2/SDL.h>

#include "colorf.hpp"
#include "widget.hpp"
#include "lalign.hpp"
#include "xmltypeset.hpp"

class LabelBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            const char8_t *label = nullptr;

            Widget::FontConfig font {};

            LabelBoard::InstAttrs attrs {};
            Widget::WADPair      parent {};
        };

    private:
        bool m_initDone = false;

    private:
        XMLTypeset m_tpset;

    public:
        LabelBoard(LabelBoard::InitArgs);

    public:
        ~LabelBoard() = default;

    public:
        void loadXML(const char *);
        void setText(const char8_t *, ...);

    public:
        void setFont(uint8_t);
        void setFontSize(uint8_t);
        void setFontStyle(uint8_t);
        void setFontColor(uint32_t);
        void setImageMaskColor(uint32_t);

    public:
        void clear()
        {
            m_tpset.clear();
        }

        bool empty() const
        {
            return m_tpset.empty();
        }

    public:
        std::string getXML() const
        {
            return m_tpset.getXML();
        }

        std::string getText() const
        {
            return m_tpset.getText();
        }

    public:
        void drawDefault(Widget::ROIMap) const override;
};
