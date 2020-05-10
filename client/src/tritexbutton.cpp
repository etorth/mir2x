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

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

void TritexButton::drawEx(int dstX, int dstY, int srcX, int srcY, int w, int h)
{
    if(auto tex = g_progUseDB->Retrieve(m_texID[m_state])){
        g_SDLDevice->DrawTexture(tex, dstX + m_offset[m_state][0], dstY + m_offset[m_state][1], srcX, srcY, w, h);
    }
}
