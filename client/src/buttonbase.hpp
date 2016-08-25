/*
 * =====================================================================================
 *
 *       Filename: button.hpp
 *        Created: 08/25/2016 04:12:57
 *  Last Modified: 08/24/2016 01:08:51
 *
 *    Description: basic button class to handle event logic only
 *                 there are three {nTexID0, nTexID1, nTexID2} texture ID's to represetn
 *                 three states, since in the PNGTexDBN, all texutres for GUI are with
 *                 nFileIndex = 0XFF, then here nTexID == 0 means there is no visual
 *                 for current state.
 *                 
 *                 I support two callbaks only: off->on and on->click
 *                 this class ask user to configure whether the on->click is triggered
 *                 at the PRESS or RELEASE event.
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
#include <cstdint>
#include <functional>

#include "log.hpp"
#include "widget.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"

class ButtonBase: public Widget
{
    public:
        ButtonBase(
                int                          nX,
                int                          nY,
                uint32_t                     nTexID0,
                uint32_t                     nTexID1,
                uint32_t                     nTexID2,
                const std::function<void()> &fnOnOver,
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
