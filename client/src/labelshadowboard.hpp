/*
 * =====================================================================================
 *
 *       Filename: labelshadowboard.hpp
 *        Created: 11/28/2020 08:59:11
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include "widget.hpp"
#include "labelboard.hpp"

class LabelShadowBoard: public WidgetGroup
{
    private:
        LabelBoard m_label;
        LabelBoard m_labelShadow;

    public:
        LabelShadowBoard(
                int            x,
                int            y,
                int            xShadowOff,
                int            yShadowOff,
                const char8_t *content         = u8"",
                uint8_t        font            = 0,
                uint8_t        fontSize        = 8,
                uint8_t        fontStyle       = 0,
                uint32_t       fontColor       = colorf::WHITE + 255,
                uint32_t       fontShadowColor = colorf::BLACK + 128,
                Widget        *widgetPtr       = nullptr,
                bool           autoDelete      = false)
            : WidgetGroup
              {
                  x,
                  y,
                  0,
                  0,
                  widgetPtr,
                  autoDelete,
              }
            , m_label
              {
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
