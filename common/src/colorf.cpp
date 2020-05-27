/*
 * =====================================================================================
 *
 *       Filename: colorf.cpp
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
#include "strf.hpp"
#include "colorf.hpp"
#include "fflerror.hpp"

uint32_t colorf::String2RGBA(const char *colorString)
{
    if(colorString == nullptr){
        throw fflerror("invalid color string: nullptr");
    }

    if(std::strlen(colorString) == 0){
        throw fflerror("invalid color string: zero-length");
    }

    // only for some color
    // check www.w3schools.com/cssref/css_colors.asp

    if(false
            || !std::strcmp(colorString, "WHITE")
            || !std::strcmp(colorString, "White")
            || !std::strcmp(colorString, "white")){
        return colorf::WHITE;
    }

    if(false
            || !std::strcmp(colorString, "RED")
            || !std::strcmp(colorString, "Red")
            || !std::strcmp(colorString, "red")){
        return colorf::RED;
    }

    if(false
            || !std::strcmp(colorString, "GREEN")
            || !std::strcmp(colorString, "Green")
            || !std::strcmp(colorString, "green")){
        return colorf::GREEN;
    }

    if(false
            || !std::strcmp(colorString, "BLUE")
            || !std::strcmp(colorString, "Blue")
            || !std::strcmp(colorString, "blue")){
        return colorf::BLUE;
    }

    if(false
            || !std::strcmp(colorString, "YELLOW")
            || !std::strcmp(colorString, "Yellow")
            || !std::strcmp(colorString, "yellow")){
        return colorf::YELLOW;
    }

    if(false
            || !std::strcmp(colorString, "PURPLE")
            || !std::strcmp(colorString, "Purple")
            || !std::strcmp(colorString, "purple")){
        return colorf::PURPLE;
    }

    // try 0XRGBA mode
    // old browser only support #RRGGBB, newer support #RRGGBBAA

    uint32_t nRGB = 0X00FFFFFF;
    if(false
            || (std::sscanf(colorString, "0X%06X", &nRGB) == 1)
            || (std::sscanf(colorString, "0x%06X", &nRGB) == 1)
            || (std::sscanf(colorString, "0x%06x", &nRGB) == 1)
            || (std::sscanf(colorString, "0X%06x", &nRGB) == 1)){
        return nRGB << 8;
    }

    uint32_t nRGBA = 0XFFFFFFFF;
    if(false
            || (std::sscanf(colorString, "0X%08X", &nRGBA) == 1)
            || (std::sscanf(colorString, "0x%08X", &nRGBA) == 1)
            || (std::sscanf(colorString, "0x%08x", &nRGBA) == 1)
            || (std::sscanf(colorString, "0X%08x", &nRGBA) == 1)){
        return nRGBA;
    }

    throw fflerror("color string not recognized: %s", colorString);
}
