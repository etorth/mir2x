/*
 * =====================================================================================
 *
 *       Filename: tritexbutton.cpp
 *        Created: 03/16/2017 15:04:17
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
#include "tritexbutton.hpp"

extern PNGTexDB *g_ProgUseDB;
extern SDLDevice *g_SDLDevice;

void TritexButton::DrawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH)
{
    if(auto pTexture = g_ProgUseDB->Retrieve(m_TexID[m_State])){
        g_SDLDevice->DrawTexture(pTexture, nDstX + m_Offset[m_State][0], nDstY + m_Offset[m_State][1], nSrcX, nSrcY, nW, nH);
    }
}
