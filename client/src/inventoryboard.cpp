/*
 * =====================================================================================
 *
 *       Filename: inventoryboard.cpp
 *        Created: 10/08/2017 19:22:30
 *  Last Modified: 11/06/2017 16:02:57
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
    , m_CloseButton(
            242,
            422,
            {0XFFFFFFFF, 0X0000001C, 0X0000001D},
            0,
            0,
            0,
            0,
            [](){},
            [this]()
            {
                Show(false);
            },
            true,
            this,
            false)
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

    auto fnDrawItem = [this](uint32_t nItem, int nDrawX, int nDrawY, int nDrawW, int nDrawH)
    {

        extern PNGTexDBN *g_CommonItemDBN;
        if(auto pTexture = g_CommonItemDBN->Retrieve(nItem)){
            int nW = -1;
            int nH = -1;
            if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nW, &nH)){
                extern SDLDevice *g_SDLDevice;
                g_SDLDevice->DrawTexture(pTexture, nDrawX + (nDrawW - nW) / 2, nDrawY + (nDrawH - nH) / 2);
            }
        }
    };

    if(auto pMyHero = m_ProcessRun->GetMyHero()){
        // 1. draw gold
        m_GoldBoard.SetText("%d", pMyHero->GetGold());

        // 2. draw items
        auto &rstInvList = pMyHero->GetInventory();
        for(auto nItemID: rstInvList){
            if(nItemID){
                int nInvGridX = 18;
                int nInvGridY = 59;
                int nInvGridW = 37;
                int nInvGridH = 37;

                int nOffX = nDstX + nInvGridX;
                int nOffY = nDstY + nInvGridY;
                fnDrawItem(nItemID, nOffX, nOffY, nInvGridW, nInvGridH);
            }
        }
    }

    m_GoldBoard.Draw();
    m_CloseButton.Draw();
}

bool InventoryBoard::ProcessEvent(const SDL_Event &rstEvent, bool *pValid)
{
    if(Show()){

        m_CloseButton.ProcessEvent(rstEvent, pValid);

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
