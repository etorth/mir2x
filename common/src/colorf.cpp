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

uint32_t colorf::string2RGBA(const char *colorString)
{
    if(colorString == nullptr){
        throw fflerror("invalid color string: nullptr");
    }

    if(std::strlen(colorString) == 0){
        throw fflerror("invalid color string: zero-length");
    }

    // only for some color
    // check www.w3schools.com/cssref/css_colors.asp

    const auto fnAlpha = [colorString]() -> uint8_t
    {
        if(const auto p = std::strstr(colorString, "+")){
            if(const auto num = std::stoi(p + 1); num < 0){
                throw fflerror("invalid color string: %s", colorString);
            }
            else{
                return std::min<int>(num, 255);
            }
        }
        return 255;
    };

    const auto fnStartWith = [colorString](const char *s) -> bool
    {
        return std::strncmp(colorString, s, std::strlen(s)) == 0;
    };

    if(false
            || fnStartWith("WHITE")
            || fnStartWith("White")
            || fnStartWith("white")){
        return colorf::WHITE + fnAlpha();
    }

    if(false
            || fnStartWith("RED")
            || fnStartWith("Red")
            || fnStartWith("red")){
        return colorf::RED + fnAlpha();
    }

    if(false
            || fnStartWith("GREEN")
            || fnStartWith("Green")
            || fnStartWith("green")){
        return colorf::GREEN + fnAlpha();
    }

    if(false
            || fnStartWith("BLUE")
            || fnStartWith("Blue")
            || fnStartWith("blue")){
        return colorf::BLUE + fnAlpha();
    }

    if(false
            || fnStartWith("YELLOW")
            || fnStartWith("Yellow")
            || fnStartWith("yellow")){
        return colorf::YELLOW + fnAlpha();
    }

    if(false
            || fnStartWith("PURPLE")
            || fnStartWith("Purple")
            || fnStartWith("purple")){
        return colorf::PURPLE + fnAlpha();
    }

    uint32_t nRGBA = 0XFFFFFFFF;
    if(false
            || (std::sscanf(colorString, "0X%08X", &nRGBA) == 1)
            || (std::sscanf(colorString, "0x%08X", &nRGBA) == 1)
            || (std::sscanf(colorString, "0x%08x", &nRGBA) == 1)
            || (std::sscanf(colorString, "0X%08x", &nRGBA) == 1)){
        return nRGBA;
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

    throw fflerror("color string not recognized: %s", colorString);
}
