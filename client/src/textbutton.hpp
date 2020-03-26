/*
 * =====================================================================================
 *
 *       Filename: textbutton.hpp
 *        Created: 08/26/2016 13:20:23
 *    Description:
 *                  simple textual button
 *                  1. extend automatically if label board is bigger than button box
 *                  2. text always get center-aligned
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
#include "buttonbase.hpp"
#include "labelboard.hpp"

class TextButton: public ButtonBase
{
    private:
        // for [3]: 0 : off
        //          1 : on
        //          2 : pressed
        //     [2]  0 : font
        //          1 : background
        uint32_t m_Color[3][2];

    private:
        uint32_t m_FrameLineColor[3];

    private:
        int m_FrameLineWidth;

    private:
        LabelBoard m_Label;

    public:
        TextButton(
                int nX,
                int nY,
                int nW,
                int nH,

                const char *szContent = "",
                uint8_t     nFont     =  0,
                uint8_t     nSize     = 10,
                uint8_t     nStyle    =  0,

                const std::function<void()> &fnOnOver  = [](){},
                const std::function<void()> &fnOnClick = [](){},

                const uint32_t (&rstColor)[2][3] =
                {
                    {
                        ColorFunc::RGBA(0XFF, 0XFF, 0XFF, 0XFF),   // front ground, off
                        ColorFunc::RGBA(0XFF, 0XFF, 0X00, 0X00),   // front ground, over
                        ColorFunc::RGBA(0XFF, 0X00, 0X00, 0X00),   // front ground, pressed
                    },

                    {
                        ColorFunc::RGBA(0X00, 0X00, 0X00, 0X00),   // background, off
                        ColorFunc::RGBA(0X00, 0X00, 0X00, 0X00),   // background, over
                        ColorFunc::RGBA(0X00, 0X00, 0X00, 0X00),   // background, pressed
                    },
                },

                const uint32_t (&rstFrameLineColor)[3] =
                {
                    ColorFunc::RGBA(0XFF, 0XFF, 0XFF, 0XFF),   // frame line, off
                    ColorFunc::RGBA(0XFF, 0X00, 0X00, 0X00),   // frame line, over
                    ColorFunc::RGBA(0XFF, 0XFF, 0X00, 0X00),   // frame line, pressed
                },

                int nFrameLineWidth = 1,

                int nOffXOnOver  = 0,
                int nOffYOnOver  = 0,
                int nOffXOnClick = 0,
                int nOffYOnClick = 0,

                bool    bOnClickDone = true,
                Widget *pWidget      = nullptr,
                bool    bFreeWidget  = false)
            : ButtonBase
              {
                  nX,
                  nY,
                  nW,
                  nH,

                  fnOnOver,
                  fnOnClick,

                  nOffXOnOver,
                  nOffYOnOver,
                  nOffXOnClick,
                  nOffYOnClick,

                  bOnClickDone,
                  pWidget,
                  bFreeWidget
              }
            , m_Color
              {
                  {rstColor[0][0], rstColor[1][0]},
                  {rstColor[0][1], rstColor[1][1]},
                  {rstColor[0][2], rstColor[1][2]},
              }
            , m_FrameLineColor
              {
                  rstFrameLineColor[0],
                  rstFrameLineColor[1],
                  rstFrameLineColor[2],
              }
            , m_FrameLineWidth(nFrameLineWidth)
            , m_Label
              {
                  0,
                  0,
                  szContent,
                  nFont,
                  nSize,
                  nStyle,
                  m_Color[0][0],
                  nullptr,
                  false,
              }
        // end of initialization list
        // put all validation in the function body
        {
            m_W = (std::max<int>)(m_W, m_Label.W());
            m_H = (std::max<int>)(m_H, m_Label.H());
        }

    public:
        void drawEx(int,    // dst x on the screen coordinate
                int,        // dst y on the screen coordinate
                int,        // src x on the widget, take top-left as origin
                int,        // src y on the widget, take top-left as origin
                int,        // size to draw
                int);       // size to draw

    public:
        void FormatText(const char *, ...);

    public:
        const LabelBoard &GetLabelBoard()
        {
            return m_Label;
        }
};
