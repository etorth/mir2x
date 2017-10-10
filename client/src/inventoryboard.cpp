/*
 * =====================================================================================
 *
 *       Filename: inventoryboard.cpp
 *        Created: 10/08/2017 19:22:30
 *  Last Modified: 10/09/2017 18:47:54
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
#include "inventoryboard.hpp"

InventoryBoard::InventoryBoard()
    : Widget(0, 0, 0, 0, nullptr, false)
{
    extern PNGTexDBN *g_ProgUseDBN;
    if(auto pTexture = g_ProgUseDBN->Retrieve(0X0000001B)){
        if(SDL_QueryTexture(pTexture, 0, 0, &m_W, &m_H)){
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
}

bool InventoryBoard::ProcessEvent(const SDL_Event &rstEvent, bool *)
{
    switch(rstEvent.type){
        case SDL_MOUSEMOTION:
            {
                if(In(rstEvent.motion.x, rstEvent.motion.y)){
                    if(rstEvent.motion.state & SDL_BUTTON_LMASK){
                        Move(rstEvent.motion.xrel, rstEvent.motion.yrel);
                        return true;
                    }
                }
                break;
            }
    }
    return false;
}
