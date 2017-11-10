/*
 * =====================================================================================
 *
 *       Filename: inventoryboard.cpp
 *        Created: 10/08/2017 19:22:30
 *  Last Modified: 11/10/2017 10:23:31
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
    , m_Pack2D()
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

void InventoryBoard::DrawItem(int nDstX, int nDstY, uint32_t nItemID, int nX, int nY, int nW, int nH)
{
    if(true
            && nItemID
            && nX >= 0
            && nY >= 0
            && nW >  0
            && nH >  0
            && nX + nW <= (int)(m_Pack2D.W())){

        extern PNGTexDBN *g_CommonItemDBN;
        if(auto pTexture = g_CommonItemDBN->Retrieve(nItemID)){

            int nItemPW = -1;
            int nItemPH = -1;
            if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nItemPW, &nItemPH)){

                int nInvGridX0 = 18;
                int nInvGridY0 = 59;
                int nInvGridPW = SYS_INVGRIDPW;
                int nInvGridPH = SYS_INVGRIDPH;

                extern SDLDevice *g_SDLDevice;
                g_SDLDevice->DrawTexture(pTexture, 
                        nDstX + nInvGridX0 + nX * nInvGridPW + (nW * nInvGridPW - nItemPW) / 2,
                        nDstY + nInvGridY0 + nY * nInvGridPH + (nH * nInvGridPH - nItemPH) / 2);
            }
        }
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
        // 1. draw gold
        m_GoldBoard.SetText("%d", pMyHero->GetGold());

        // 2. draw items
        auto &rstInvList = pMyHero->GetInventory();
        if(rstInvList.size() != m_PackBinList.size()){
            Repack();
        }

        for(auto &rstBin: m_PackBinList){
            DrawItem(nDstX, nDstY, rstBin.ID, rstBin.X, rstBin.Y, rstBin.W, rstBin.H);
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

bool InventoryBoard::Repack()
{
    if(auto pMyHero = m_ProcessRun->GetMyHero()){
        const auto &rstItemList = pMyHero->GetInventory();
        if(!rstItemList.empty()){
            m_PackBinList.clear();
            for(auto nItemID: rstItemList){
                extern PNGTexDBN *g_CommonItemDBN;
                if(auto pTexture = g_CommonItemDBN->Retrieve(nItemID)){
                    int nItemPW = -1;
                    int nItemPH = -1;
                    if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nItemPW, &nItemPH)){
                        m_PackBinList.emplace_back(nItemID, -1, -1,
                                ((nItemPW + SYS_INVGRIDPW - 1) / SYS_INVGRIDPW),
                                ((nItemPH + SYS_INVGRIDPH - 1) / SYS_INVGRIDPH));
                    }
                }else{
                    // make it as 1x1
                    // we may have items without gfx resource
                    m_PackBinList.emplace_back(nItemID, -1, -1, 1, 1);
                }
            }

            switch(m_Pack2D.Pack(&m_PackBinList)){
                case 1:
                    {
                        return true;
                    }
                default:
                    {
                        return false;
                    }
            }
        }
        return true;
    }
    return false;
}

bool InventoryBoard::Add(uint32_t nItemID)
{
    int nW = 1;
    int nH = 1;

    extern PNGTexDBN *g_CommonItemDBN;
    if(auto pTexture = g_CommonItemDBN->Retrieve(nItemID)){
        int nItemPW = -1;
        int nItemPH = -1;
        if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nItemPW, &nItemPH)){
            nW = ((nItemPW + SYS_INVGRIDPW - 1) / SYS_INVGRIDPW);
            nH = ((nItemPH + SYS_INVGRIDPH - 1) / SYS_INVGRIDPH);
        }
    }

    PackBin stBin(nItemID, -1, -1, nW, nH);
    switch(m_Pack2D.Add(&stBin, 1)){
        case 1:
            {
                return true;
            }
        default:
            {
                return false;
            }
    }
}

bool InventoryBoard::Remove(uint32_t nItemID, int nX, int nY)
{
    for(size_t nIndex = 0; nIndex < m_PackBinList.size(); ++nIndex){
        if(true
                && m_PackBinList[nIndex].X == nX
                && m_PackBinList[nIndex].Y == nY
                && m_PackBinList[nIndex].ID == nItemID){
            m_Pack2D.Remove(m_PackBinList[nIndex]);
            m_PackBinList.erase(m_PackBinList.begin() + nIndex);
            return true;
        }
    }
    return false;
}
