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
        XMLTypeset m_tpset;

    public:
        LabelBoard(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                const char8_t * = nullptr,

                uint8_t =  0,
                uint8_t = 10,
                uint8_t =  0,

                uint32_t = colorf::WHITE + colorf::A_SHF(255),

                Widget * = nullptr,
                bool     = false);

    public:
        ~LabelBoard() = default;

    public:
        void loadXML(const char *);
        void setText(const char8_t *, ...);

    public:
        std::string getText(bool textOnly) const
        {
            return m_tpset.getText(textOnly);
        }

    public:
        void setFont(uint8_t argFont)
        {
            m_tpset.setFont(argFont);
            m_tpset.build();
            setSize(m_tpset.px() + m_tpset.pw(), m_tpset.py() + m_tpset.ph());
        }

        void setFontSize(uint8_t argFontSize)
        {
            m_tpset.setFontSize(argFontSize);
            m_tpset.build();
            setSize(m_tpset.px() + m_tpset.pw(), m_tpset.py() + m_tpset.ph());
        }

        void setFontStyle(uint8_t argFontStyle)
        {
            m_tpset.setFontStyle(argFontStyle);
            m_tpset.build();
            setSize(m_tpset.px() + m_tpset.pw(), m_tpset.py() + m_tpset.ph());
        }

        void setFontColor(uint32_t argFontColor)
        {
            m_tpset.setFontColor(argFontColor);
        }

    public:
        void clear()
        {
            m_tpset.clear();
            setSize(0, 0);
        }

    public:
        std::string getXML() const
        {
            return m_tpset.getXML();
        }

    public:
        void drawEx(int argDstX, int argDstY, int argSrcX, int argSrcY, int argSrcW, int argSrcH) const override
        {
            m_tpset.drawEx(argDstX, argDstY, argSrcX, argSrcY, argSrcW, argSrcH);
        }

    public:
        void setImageMaskColor(uint32_t color)
        {
            m_tpset.setImageMaskColor(color);
        }

    public:
        bool empty() const
        {
            return m_tpset.empty();
        }
};
