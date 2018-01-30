/*
 * =====================================================================================
 *
 *       Filename: colorfunc.cpp
 *        Created: 03/31/2016 19:48:57
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

#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include "colorfunc.hpp"

SDL_Color ColorFunc::RGBA2Color(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
{
    SDL_Color stColor;
    stColor.r = nR;
    stColor.g = nG;
    stColor.b = nB;
    stColor.a = nA;
    return stColor;
}

SDL_Color ColorFunc::ARGB2Color(uint8_t nA, uint8_t nR, uint8_t nG, uint8_t nB)
{
    return RGBA2Color(nR, nG, nB, nA);
}

SDL_Color ColorFunc::RGBA2Color(uint32_t nRGBA)
{
    return RGBA2Color((nRGBA & 0XFF000000) >> 24, (nRGBA & 0X00FF0000) >> 16, (nRGBA & 0X0000FF00) >> 8, (nRGBA & 0X000000FF));
}

SDL_Color ColorFunc::ARGB2Color(uint32_t nARGB)
{
    return ARGB2Color((nARGB & 0XFF000000) >> 24, (nARGB & 0X00FF0000) >> 16, (nARGB & 0X0000FF00) >> 8, (nARGB & 0X000000FF));
}

uint32_t ColorFunc::Color2RGBA(const SDL_Color &rstColor)
{
    return 0
        | ((uint32_t)(rstColor.r) << 24)
        | ((uint32_t)(rstColor.g) << 16)
        | ((uint32_t)(rstColor.b) <<  8)
        | ((uint32_t)(rstColor.a) <<  0);
}

uint32_t ColorFunc::Color2ARGB(const SDL_Color &rstColor)
{
    uint32_t nRGBA = Color2RGBA(rstColor);
    return ((nRGBA & 0X000000FF) << 24) | ((nRGBA & 0XFFFFFF00) >> 8);
}

SDL_Color ColorFunc::RenderColor(const SDL_Color &rstDstColor, const SDL_Color &rstSrcColor)
{
    SDL_Color stRetColor;
    stRetColor.r = std::min<uint8_t>(255, std::lround((rstSrcColor.a / 255.0) * rstSrcColor.r + (1.0 - rstSrcColor.a / 255.0) * rstDstColor.r));
    stRetColor.g = std::min<uint8_t>(255, std::lround((rstSrcColor.a / 255.0) * rstSrcColor.g + (1.0 - rstSrcColor.a / 255.0) * rstDstColor.g));
    stRetColor.b = std::min<uint8_t>(255, std::lround((rstSrcColor.a / 255.0) * rstSrcColor.b + (1.0 - rstSrcColor.a / 255.0) * rstDstColor.b));
    stRetColor.a = std::min<uint8_t>(255, std::lround((rstSrcColor.a *   1.0)                 + (1.0 - rstSrcColor.a / 255.0) * rstDstColor.a));
    return stRetColor;
}

uint32_t ColorFunc::RenderRGBA(uint32_t nDstColor, uint32_t nSrcColor)
{
    return Color2RGBA(RenderColor(RGBA2Color(nDstColor), RGBA2Color(nSrcColor)));
}

uint32_t ColorFunc::RenderARGB(uint32_t nDstColor, uint32_t nSrcColor)
{
    return Color2ARGB(RenderColor(ARGB2Color(nDstColor), ARGB2Color(nSrcColor)));
}

bool ColorFunc::String2Color(SDL_Color *pstColor, const char *szText)
{
    if(!szText || std::strlen(szText) == 0){ return false; }

    bool bFind = true;
    SDL_Color stColor {0X00, 0X00, 0X00, 0XFF};
    if(false
            || !std::strcmp(szText, "RED")
            || !std::strcmp(szText, "Red")
            || !std::strcmp(szText, "red")){
        stColor.r = 0XFF;
    }else if(false
            || !std::strcmp(szText, "GREEN")
            || !std::strcmp(szText, "Green")
            || !std::strcmp(szText, "green")){
        stColor.g = 0XFF;
    }else if(false
            || !std::strcmp(szText, "BLUE")
            || !std::strcmp(szText, "Blue")
            || !std::strcmp(szText, "blue")){
        stColor.b = 0XFF;
    }else if(false
            || !std::strcmp(szText, "YELLOW")
            || !std::strcmp(szText, "Yellow")
            || !std::strcmp(szText, "yellow")){
        stColor.r = 0XFF;
        stColor.g = 0XFF;
    }else if(false
            || !std::strcmp(szText, "PURPLE")
            || !std::strcmp(szText, "Purple")
            || !std::strcmp(szText, "purple")){
        stColor.r = 0X80;
        stColor.b = 0X80;
    }else{
        // try "0XFF00FF00FF" mode
        int nRes = -1;
        uint32_t nARGB = 0XFFFFFFFF;

        if(false
                || ((nRes = std::sscanf(szText, "0X%08X", &nARGB)) == 1)
                || ((nRes = std::sscanf(szText, "0x%08X", &nARGB)) == 1)
                || ((nRes = std::sscanf(szText, "0x%08x", &nARGB)) == 1)
                || ((nRes = std::sscanf(szText, "0X%08x", &nARGB)) == 1)){
            stColor = ARGB2Color(nARGB);
        }else{
            bFind = false;
        }
    }

    if(bFind && pstColor){ *pstColor = stColor; }
    return bFind;
}
