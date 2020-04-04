/*
 * =====================================================================================
 *
 *       Filename: inventoryboard.cpp
 *        Created: 10/08/2017 19:22:30
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

#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"

extern PNGTexDB *g_ProgUseDB;
extern PNGTexDB *g_ProgUseDB;
extern PNGTexDB *g_CommonItemDB;
extern SDLDevice *g_SDLDevice;

InventoryBoard::InventoryBoard(int nX, int nY, ProcessRun *pRun, widget *pwidget, bool bAutoFree)
    : widget(nX, nY, 0, 0, pwidget, bAutoFree)
    , m_goldBoard(70, 403, "0", 0, 15, 0, ColorFunc::RGBA(0XFF, 0XFF, 0X00, 0X00), this)
    , m_closeButton(
            242,
            422,
            {0XFFFFFFFF, 0X0000001C, 0X0000001D},
            [](){},
            [this]()
            {
                show(false);
            },
            0,
            0,
            0,
            0,
            true,
            this,
            false)
    , m_ProcessRun(pRun)
{
    if(auto pTexture = g_ProgUseDB->Retrieve(0X0000001B)){
        SDL_QueryTexture(pTexture, nullptr, nullptr, &m_w, &m_h);
    }
}

void InventoryBoard::drawItem(int nDstX, int nDstY, const PackBin &rstBin)
{
    if(true
            && rstBin
            && rstBin.X >= 0
            && rstBin.Y >= 0
            && rstBin.W >  0
            && rstBin.H >  0){

        if(auto pTexture = g_CommonItemDB->Retrieve(rstBin.ID)){

            int nItemPW = -1;
            int nItemPH = -1;
            if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nItemPW, &nItemPH)){

                const int nInvGridX0 = 18;
                const int nInvGridY0 = 59;

                g_SDLDevice->DrawTexture(pTexture, 
                        nDstX + nInvGridX0 + rstBin.X * SYS_INVGRIDPW + (rstBin.W * SYS_INVGRIDPW - nItemPW) / 2,
                        nDstY + nInvGridY0 + rstBin.Y * SYS_INVGRIDPH + (rstBin.H * SYS_INVGRIDPH - nItemPH) / 2);
            }
        }
    }
}

void InventoryBoard::drawEx(int nDstX, int nDstY, int, int, int, int)
{
    if(auto pTexture = g_ProgUseDB->Retrieve(0X0000001B)){
        g_SDLDevice->DrawTexture(pTexture, nDstX, nDstY);
    }

    if(auto pMyHero = m_ProcessRun->GetMyHero()){
        // 1. draw gold
        m_goldBoard.setText("%d", pMyHero->GetGold());

        // 2. draw all items
        for(auto &rstBin: pMyHero->getInvPack().GetPackBinList()){
            drawItem(nDstX, nDstY, rstBin);
        }
    }

    m_goldBoard.draw();
    m_closeButton.draw();
}

bool InventoryBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    if(!show()){
        return false;
    }

    if(m_closeButton.processEvent(event, valid)){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if(in(event.motion.x, event.motion.y) && (event.motion.state & SDL_BUTTON_LMASK)){
                    moveBy(event.motion.xrel, event.motion.yrel);
                    return true;
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}
