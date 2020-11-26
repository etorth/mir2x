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

#include "sysconst.hpp"
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

void TritexButton::initButtonSize()
{
    int maxW = 0;
    int maxH = 0;
    for(const int state: {0, 1, 2}){
        if(m_texID[state] != SYS_TEXNIL){
            if(auto texPtr = g_progUseDB->Retrieve(m_texID[state])){
                const auto [texCurrW, texCurrH] = SDLDevice::getTextureSize(texPtr);
                maxW = (std::max<int>)(texCurrW, maxW);
                maxH = (std::max<int>)(texCurrH, maxH);
            }
        }
    }

    // we allow buttons without any valid texture, in that case some extra work
    // can be done for special drawing
    m_w = maxW;
    m_h = maxH;
}
