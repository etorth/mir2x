#pragma once
#include "widget.hpp"
#include "labelboard.hpp"

class LabelShadowBoard: public Widget
{
    private:
        LabelBoard m_labelShadow;
        LabelBoard m_label;

    public:
        LabelShadowBoard(
                dir8_t argDir,
                int    argX,
                int    argY,

                int argXShadowOff,
                int argYShadowOff,

                const char8_t *argContent = u8"",

                uint8_t  argFont            = 0,
                uint8_t  argFontSize        = 8,
                uint8_t  argFontStyle       = 0,
                uint32_t argFontColor       = colorf::WHITE + colorf::A_SHF(255),
                uint32_t argFontShadowColor = colorf::BLACK + colorf::A_SHF(128),

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  argDir,
                  argX,
                  argY,
                  {},
                  {},
                  {},

                  argParent,
                  argAutoDelete,
              }

            , m_labelShadow
              {
                  DIR_UPLEFT,
                  std::max<int>(0, argXShadowOff),
                  std::max<int>(0, argYShadowOff),
                  argContent,
                  argFont,
                  argFontSize,
                  argFontStyle,
                  argFontShadowColor,
                  this,
                  false,
              }

            , m_label
              {
                  DIR_UPLEFT,
                  0,
                  0,
                  argContent,
                  argFont,
                  argFontSize,
                  argFontStyle,
                  argFontColor,
                  this,
                  false,
              }
        {
            m_labelShadow.setImageMaskColor(0);
        }

    public:
        void setText(const std::u8string &s)
        {
            m_labelShadow.setText(s.c_str());
            m_label      .setText(s.c_str());
        }
};
