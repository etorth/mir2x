/*
 * =====================================================================================
 *
 *       Filename: colorfunc.cpp
 *        Created: 03/31/2016 19:48:57
 *  Last Modified: 05/20/2017 21:50:48
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

#include <cstdio>
#include <cstring>
#include "colorfunc.hpp"

SDL_Color ColorFunc::MakeColor(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
{
    SDL_Color stColor;
    stColor.r = nR;
    stColor.g = nG;
    stColor.b = nB;
    stColor.a = nA;
    return stColor;
}

uint32_t ColorFunc::Color2U32RGBA(const SDL_Color &rstColor)
{
    uint32_t nRes = 0;
    nRes += ((uint32_t)(rstColor.r) << 24);
    nRes += ((uint32_t)(rstColor.g) << 16);
    nRes += ((uint32_t)(rstColor.b) <<  8);
    nRes += ((uint32_t)(rstColor.a) <<  0);

    return nRes;
}

uint32_t ColorFunc::Color2U32ARGB(const SDL_Color &rstColor)
{
    uint32_t nRes = 0;
    nRes += ((uint32_t)(rstColor.a) << 24);
    nRes += ((uint32_t)(rstColor.r) << 16);
    nRes += ((uint32_t)(rstColor.g) <<  8);
    nRes += ((uint32_t)(rstColor.b) <<  0);

    return nRes;
}

SDL_Color ColorFunc::U32RGBA2Color(uint32_t nColor)
{
    SDL_Color stColor;
    stColor.r = ((nColor & 0XFF000000) >> 24);
    stColor.g = ((nColor & 0X00FF0000) >> 16);
    stColor.b = ((nColor & 0X0000FF00) >>  8);
    stColor.a = ((nColor & 0X000000FF) >>  0);

    return stColor;
}

SDL_Color ColorFunc::U32ARGB2Color(uint32_t nColor)
{
    SDL_Color stColor;
    stColor.a = ((nColor & 0XFF000000) >> 24);
    stColor.r = ((nColor & 0X00FF0000) >> 16);
    stColor.g = ((nColor & 0X0000FF00) >>  8);
    stColor.b = ((nColor & 0X000000FF) >>  0);

    return stColor;
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
            stColor = U32ARGB2Color(nARGB);
        }else{
            bFind = false;
        }
    }

    if(bFind && pstColor){ *pstColor = stColor; }
    return bFind;
}
