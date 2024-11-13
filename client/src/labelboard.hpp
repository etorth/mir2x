#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <SDL2/SDL.h>

#include "widget.hpp"
#include "lalign.hpp"
#include "xmltypeset.hpp"
#include "colorf.hpp"

class LabelBoard: public Widget
{
    private:
        XMLTypeset m_tpset;

    public:
        LabelBoard(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                const char8_t *argContent    = nullptr,
                uint8_t        argFont       = 0,
                uint8_t        argFontSize   = 10,
                uint8_t        argFontStyle  = 0,
                uint32_t       argFontColor  = colorf::WHITE + colorf::A_SHF(255),

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),
                  0,
                  0,

                  {},

                  argParent,
                  argAutoDelete,
              }

            , m_tpset
              {
                  0,
                  LALIGN_LEFT,
                  false,
                  argFont,
                  argFontSize,
                  argFontStyle,
                  argFontColor,
              }
        {
            setText(u8"%s", argContent ? argContent : u8"");
        }

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
        void setFont(uint8_t nFont)
        {
            m_tpset.setFont(nFont);
            m_tpset.build();
            setSize(m_tpset.px() + m_tpset.pw(), m_tpset.py() + m_tpset.ph());
        }

        void setFontSize(uint8_t nFontSize)
        {
            m_tpset.setFontSize(nFontSize);
            m_tpset.build();
            setSize(m_tpset.px() + m_tpset.pw(), m_tpset.py() + m_tpset.ph());
        }

        void setFontStyle(uint8_t nFontStyle)
        {
            m_tpset.setFontStyle(nFontStyle);
            m_tpset.build();
            setSize(m_tpset.px() + m_tpset.pw(), m_tpset.py() + m_tpset.ph());
        }

        void setFontColor(uint32_t nFontColor)
        {
            m_tpset.setFontColor(nFontColor);
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
