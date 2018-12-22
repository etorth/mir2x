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
#include "strfunc.hpp"
#include "colorfunc.hpp"

uint32_t ColorFunc::String2RGBA(const char *szColorString)
{
    if(szColorString == nullptr){
        throw std::invalid_argument(str_fflprintf(": Invalid color string: nullptr"));
    }

    if(std::strlen(szColorString) == 0){
        throw std::invalid_argument(str_fflprintf(": Invalid color string: zero-length"));
    }

    // only for some color
    // check www.w3schools.com/cssref/css_colors.asp

    if(false
            || !std::strcmp(szColorString, "WHITE")
            || !std::strcmp(szColorString, "White")
            || !std::strcmp(szColorString, "white")){
        return ColorFunc::WHITE;
    }

    if(false
            || !std::strcmp(szColorString, "RED")
            || !std::strcmp(szColorString, "Red")
            || !std::strcmp(szColorString, "red")){
        return ColorFunc::RED;
    }

    if(false
            || !std::strcmp(szColorString, "GREEN")
            || !std::strcmp(szColorString, "Green")
            || !std::strcmp(szColorString, "green")){
        return ColorFunc::GREEN;
    }

    if(false
            || !std::strcmp(szColorString, "BLUE")
            || !std::strcmp(szColorString, "Blue")
            || !std::strcmp(szColorString, "blue")){
        return ColorFunc::BLUE;
    }

    if(false
            || !std::strcmp(szColorString, "YELLOW")
            || !std::strcmp(szColorString, "Yellow")
            || !std::strcmp(szColorString, "yellow")){
        return ColorFunc::YELLOW;
    }

    if(false
            || !std::strcmp(szColorString, "PURPLE")
            || !std::strcmp(szColorString, "Purple")
            || !std::strcmp(szColorString, "purple")){
        return ColorFunc::PURPLE;
    }

    // try 0XRGBA mode
    // old browser only support #RRGGBB, newer support #RRGGBBAA

    uint32_t nRGB = 0X00FFFFFF;
    if(false
            || (std::sscanf(szColorString, "0X%06X", &nRGB) == 1)
            || (std::sscanf(szColorString, "0x%06X", &nRGB) == 1)
            || (std::sscanf(szColorString, "0x%06x", &nRGB) == 1)
            || (std::sscanf(szColorString, "0X%06x", &nRGB) == 1)){
        return nRGB << 8;
    }

    uint32_t nRGBA = 0XFFFFFFFF;
    if(false
            || (std::sscanf(szColorString, "0X%08X", &nRGBA) == 1)
            || (std::sscanf(szColorString, "0x%08X", &nRGBA) == 1)
            || (std::sscanf(szColorString, "0x%08x", &nRGBA) == 1)
            || (std::sscanf(szColorString, "0X%08x", &nRGBA) == 1)){
        return nRGBA;
    }

    throw std::invalid_argument(str_fflprintf(": color string not recognized: %s", szColorString));
}
