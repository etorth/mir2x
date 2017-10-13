/*
 * =====================================================================================
 *
 *       Filename: tritexbutton.cpp
 *        Created: 03/16/2017 15:04:17
 *  Last Modified: 10/13/2017 00:58:23
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
    extern PNGTexDBN *g_ProgUseDBN;
    extern SDLDevice *g_SDLDevice;

    if(auto pTexture = g_ProgUseDBN->Retrieve(m_TexIDV[m_State])){
        g_SDLDevice->DrawTexture(pTexture, nDstX + m_Offset[m_State][0], nDstY + m_Offset[m_State][1], nSrcX, nSrcY, nW, nH);
    }
}
