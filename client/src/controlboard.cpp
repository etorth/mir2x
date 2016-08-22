/*
 * =====================================================================================
 *
 *       Filename: controlboard.cpp
 *        Created: 08/21/2016 04:12:57
 *  Last Modified: 08/21/2016 22:01:48
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

#include <stdexcept>
#include <algorithm>
#include <functional>

#include "log.hpp"
#include "sdldevice.hpp"
#include "pngtexdbn.hpp"
#include "controlboard.hpp"

ControlBoard::ControlBoard(int nX, int nY, Widget *pWidget, bool bAutoDelete)
    : Widget(nX, nY, 0, 0, pWidget, bAutoDelete)
{
}

void ControlBoard::Draw(int, int)
{
    extern PNGTexDBN *g_PNGTexDBN;
    extern SDLDevice *g_SDLDevice;

    g_SDLDevice->DrawTexture(g_PNGTexDBN->Retrieve((0XFF << 16) + 0X0012),   0, 466);
    g_SDLDevice->DrawTexture(g_PNGTexDBN->Retrieve((0XFF << 16) + 0X0013), 178, 448);
}

bool ControlBoard::ProcessEvent(const SDL_Event &rstEvent, bool *bValid)
{
    if(bValid && !(*bValid)){ return false; }

    switch(rstEvent.type){
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEMOTION:
        default:
            return false;
    }
    return false;
}
