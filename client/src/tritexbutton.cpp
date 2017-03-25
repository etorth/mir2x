/*
 * =====================================================================================
 *
 *       Filename: tritexbutton.cpp
 *        Created: 03/16/2017 15:04:17
 *  Last Modified: 03/24/2017 18:05:21
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

#include "sdldevice.hpp"
#include "pngtexdbn.hpp"
#include "tritexbutton.hpp"

void TritexButton::DrawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH)
{
    extern PNGTexDBN *g_PNGTexDBN;
    extern SDLDevice *g_SDLDevice;

    g_SDLDevice->DrawTexture(g_PNGTexDBN->Retrieve(m_TexIDV[m_State]), nDstX, nDstY, nSrcX, nSrcY, nW, nH);
}
