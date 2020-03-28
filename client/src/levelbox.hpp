/*
 * =====================================================================================
 *
 *       Filename: levelbox.hpp
 *        Created: 03/28/2020 05:43:45
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
#include <functional>
#include "widget.hpp"
#include "colorfunc.hpp"
#include "labelboard.hpp"

class levelBox: public Widget
{
    private:
        labelBoard m_label;

    private:
        std::function<void(int)> m_onDrag;
        std::function<void()> m_onDoubleClick;

    public:
        levelBox(
                int x,
                int y,

                std::function<void(int)> onDrag,
                std::function<void()> onDoubleClick,

                Widget *parent,
                bool autoDelete)
            : Widget(x, y, 0, 0, parent, autoDelete)
            , m_label
              {
                  0,
                  0,
                  "",
                  0,
                  12,
                  0,
                  ColorFunc::YELLOW + 128,
                  this,
                  false,
              }
            , m_onDrag(onDrag)
            , m_onDoubleClick(onDoubleClick)
        {
            setLevel(0);
        }

    public:
        bool processEvent(const SDL_Event &, bool);

    public:
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) override
        {
            m_label.drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
        }

    public:
        void setLevel(int level)
        {
            m_label.setText("%d", level);

            m_W = std::max<int>(m_label.W(), 18);
            m_H = std::max<int>(m_label.H(), 10);

            m_label.moveTo((W() - m_label.W()) / 2,(H() - m_label.H()) / 2);
        }
};
