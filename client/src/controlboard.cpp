/*
 * =====================================================================================
 *
 *       Filename: controlboard.cpp
 *        Created: 08/21/2016 04:12:57
 *  Last Modified: 05/25/2017 00:08:51
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
    , m_CmdLine(
            100,
            100,
            10,
            10,
            false,
            100)
{
}

void ControlBoard::Update(double fMS)
{
    m_CmdLine.Update(fMS);
}

void ControlBoard::DrawEx(int, int, int, int, int, int)
{
    extern PNGTexDBN *g_PNGTexDBN;
    extern SDLDevice *g_SDLDevice;

    g_SDLDevice->DrawTexture(g_PNGTexDBN->Retrieve((0XFF << 16) + 0X0012),   0, 466);
    g_SDLDevice->DrawTexture(g_PNGTexDBN->Retrieve((0XFF << 16) + 0X0013), 178, 448);

    m_CmdLine.Draw();
}

bool ControlBoard::ProcessEvent(const SDL_Event &rstEvent, bool *bValid)
{
    if(bValid && !(*bValid)){ return false; }
    if(m_CmdLine.ProcessEvent(rstEvent, bValid)){ return false; }

    switch(rstEvent.type){
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEMOTION:
        default:
            return false;
    }
    return false;
}
