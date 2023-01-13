#pragma once
#include "widget.hpp"
#include "labelboard.hpp"

class LabelShadowBoard: public WidgetContainer
{
    private:
        LabelBoard m_label;
        LabelBoard m_labelShadow;

    public:
        LabelShadowBoard(
                dir8_t         dir,
                int            x,
                int            y,
                int            xShadowOff,
                int            yShadowOff,
                const char8_t *content         = u8"",
                uint8_t        font            = 0,
                uint8_t        fontSize        = 8,
                uint8_t        fontStyle       = 0,
                uint32_t       fontColor       = colorf::WHITE + colorf::A_SHF(255),
                uint32_t       fontShadowColor = colorf::BLACK + colorf::A_SHF(128),
                Widget        *widgetPtr       = nullptr,
                bool           autoDelete      = false)
            : WidgetContainer
              {
                  dir,
                  x,
                  y,
                  0,
                  0,
                  widgetPtr,
                  autoDelete,
              }
            , m_label
              {
                  DIR_UPLEFT,
                  0,
                  0,
                  content,
                  font,
                  fontSize,
                  fontStyle,
                  fontColor,
                  this,
                  false,
              }
            , m_labelShadow
              {
                  DIR_UPLEFT,
                  std::max<int>(0, xShadowOff),
                  std::max<int>(0, yShadowOff),
                  content,
                  font,
                  fontSize,
                  fontStyle,
                  fontShadowColor,
                  this,
                  false,
              }
        {
            m_labelShadow.setImageMaskColor(0);
            updateSize();
        }

    public:
        void setText(const std::u8string &s)
        {
            m_label      .setText(s.c_str());
            m_labelShadow.setText(s.c_str());
            updateSize();
        }

    private:
        void updateSize()
        {
            m_w = m_labelShadow.dx() + m_labelShadow.w();
            m_h = m_labelShadow.dy() + m_labelShadow.h();
        }
};
