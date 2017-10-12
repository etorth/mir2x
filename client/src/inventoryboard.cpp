/*
 * =====================================================================================
 *
 *       Filename: inventoryboard.cpp
 *        Created: 10/08/2017 19:22:30
 *  Last Modified: 10/11/2017 23:19:35
 *
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

#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"

InventoryBoard::InventoryBoard(int nX, int nY, ProcessRun *pRun, Widget *pWidget, bool bAutoFree)
    : Widget(nX, nY, 0, 0, pWidget, bAutoFree)
    , m_GoldBoard(70, 408, "", 0, 15, 0, {0XFF, 0XFF, 0X00, 0X00}, this, false)
    , m_ProcessRun(pRun)
{
    extern PNGTexDBN *g_ProgUseDBN;
    if(auto pTexture = g_ProgUseDBN->Retrieve(0X0000001B)){
        SDL_QueryTexture(pTexture, nullptr, nullptr, &m_W, &m_H);
    }
}

void InventoryBoard::DrawEx(int nDstX, int nDstY, int, int, int, int)
{
    extern SDLDevice *g_SDLDevice;
    extern PNGTexDBN *g_ProgUseDBN;
    if(auto pTexture = g_ProgUseDBN->Retrieve(0X0000001B)){
        g_SDLDevice->DrawTexture(pTexture, nDstX, nDstY);
    }

    if(auto pMyHero = m_ProcessRun->GetMyHero()){
        m_GoldBoard.SetText("%d", pMyHero->GetGold());
    }

    m_GoldBoard.Draw();
}

bool InventoryBoard::ProcessEvent(const SDL_Event &rstEvent, bool *pValid)
{
    if(Show()){
        switch(rstEvent.type){
            case SDL_MOUSEMOTION:
                {
                    if(pValid && *pValid){
                        if(In(rstEvent.motion.x, rstEvent.motion.y)){
                            if(rstEvent.motion.state & SDL_BUTTON_LMASK){
                                Move(rstEvent.motion.xrel, rstEvent.motion.yrel);
                                return true;
                            }
                        }
                    }
                    break;
                }
        }
    }
    return false;
}
