/*
 * =====================================================================================
 *
 *       Filename: button.hpp
 *        Created: 08/21/2015 04:12:57
 *  Last Modified: 08/24/2016 01:08:51
 *
 *    Description: Button, texture id should be baseID + [0, 1, 2]
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
#include "log.hpp"
#include "widget.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"

#include <cstdint>
#include <functional>

class Button: public Widget
{
    public:
        Button(
                int                          nX,
                int                          nY,
                uint8_t                      nFileIndex,
                uint16_t                     nImageIndex,
                const std::function<void()> &fnOnClick   = [](){},
                Widget                      *pWidget     = nullptr,
                bool                         bFreeWidget = false):
            Widget(nX, nY, 0, 0, pWidget, bFreeWidget)
            , m_BaseID((((uint32_t)nFileIndex) << 16) + nImageIndex)
            , m_State(0)
            , m_OnClick(fnOnClick)
        {
            extern Log       *g_Log;
            extern PNGTexDBN *g_PNGTexDBN;

            auto pTexture = g_PNGTexDBN->Retrieve(m_BaseID);
            if(pTexture){
                if(SDL_QueryTexture(pTexture, nullptr, nullptr, &m_W, &m_H)){
                    g_Log->AddLog(LOGTYPE_INFO, "Button(%d, %d): X = %d, Y = %d, W = %d, H = %d", nFileIndex, nImageIndex, m_X, m_Y, m_W, m_H);
                }
            }
        }
        virtual ~Button() = default;

    public:
        using Widget::Draw;
        void Draw(int, int);
        bool ProcessEvent(const SDL_Event &, bool *);

    private:
        // 0: normal
        // 1: on
        // 2: pressed
        uint32_t               m_BaseID;
        int                    m_State;
        std::function<void()>  m_OnClick;
};
