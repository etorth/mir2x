/*
 * =====================================================================================
 *
 *       Filename: buttonbase.hpp
 *        Created: 08/25/2016 04:12:57
 *  Last Modified: 07/04/2017 12:39:32
 *
 *    Description: basic button class to handle event logic only
 *
 *                 1. use g_ProgUse for texture storage
 *                 2. use three textures for three states
 *
 *                 I support two callbaks only: off->on and on->click
 *                 this class ask user to configure whether the on->click is triggered
 *                 at the PRESS or RELEASE event.
 *
 *                 I require the size of three textures should be approximately same to
 *                 avoid unnecessay complexity
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

#include "widget.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"

class ButtonBase: public Widget
{
    protected:
        // 0: off
        // 1: over
        // 2: pressed
        int                    m_State;
        bool                   m_OnClickDone;
        uint32_t               m_TexIDV[3];
        std::function<void()>  m_OnOver;
        std::function<void()>  m_OnClick;
        
    public:
        ButtonBase(
                int                          nX,
                int                          nY,
                uint32_t                     nTexID0,
                uint32_t                     nTexID1,
                uint32_t                     nTexID2,
                const std::function<void()> &fnOnOver     = [](){},
                const std::function<void()> &fnOnClick    = [](){},
                bool                         bOnClickDone = true,
                Widget                      *pWidget      = nullptr,
                bool                         bFreeWidget  = false)
            : Widget(nX, nY, 0, 0, pWidget, bFreeWidget)
            , m_State(0)
            , m_OnClickDone(bOnClickDone)
            , m_TexIDV {nTexID0, nTexID1, nTexID2}
            , m_OnOver(fnOnOver)
            , m_OnClick(fnOnClick)
        {
            int nW = 0;
            int nH = 0;
            for(int nState = 0; nState < 2; ++nState){
                if(m_TexIDV[nState]){
                    extern PNGTexDBN *g_PNGTexDBN;
                    if(auto pTexture = g_PNGTexDBN->Retrieve(m_TexIDV[nState])){
                        int nCurrW, nCurrH;
                        if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nCurrW, &nCurrH)){
                            nW = std::max(nCurrW, nW);
                            nH = std::max(nCurrH, nH);
                        }
                    }
                }
            }

            // we allow buttons without any valid texture, in that case some extra work
            // can be done for special drawing
            m_W = nW;
            m_H = nH;
        }
        virtual ~ButtonBase() = default;

    public:
        bool ProcessEvent(const SDL_Event &, bool *);
};
