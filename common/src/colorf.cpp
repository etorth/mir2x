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
    fflassert(str_haschar(colorString));

    // only for some color
    // check www.w3schools.com/cssref/css_colors.asp

    const auto fn_A_SHF = [colorString]() -> uint32_t
    {
        if(const auto p = std::strstr(colorString, "+")){
            int alpha = 0;
            try{
                alpha = std::stoi(p + 1);
            }
            catch(...){
                alpha = -1;
            }

            if(alpha < 0){
                throw fflerror("invalid color string: %s", colorString);
            }
            else{
                return A_SHF(std::min<int>(alpha, 255));
            }
        }
        return A_SHF(255);
    };

    const auto fnStartWith = [colorString](const char *s) -> bool
    {
        return std::strncmp(colorString, s, std::strlen(s)) == 0;
    };

    if(false
            || fnStartWith("WHITE")
            || fnStartWith("White")
            || fnStartWith("white")){
        return colorf::WHITE + fn_A_SHF();
    }

    if(false
            || fnStartWith("RED")
            || fnStartWith("Red")
            || fnStartWith("red")){
        return colorf::RED + fn_A_SHF();
    }

    if(false
            || fnStartWith("GREEN")
            || fnStartWith("Green")
            || fnStartWith("green")){
        return colorf::GREEN + fn_A_SHF();
    }

    if(false
            || fnStartWith("BLUE")
            || fnStartWith("Blue")
            || fnStartWith("blue")){
        return colorf::BLUE + fn_A_SHF();
    }

    if(false
            || fnStartWith("YELLOW")
            || fnStartWith("Yellow")
            || fnStartWith("yellow")){
        return colorf::YELLOW + fn_A_SHF();
    }

    if(false
            || fnStartWith("MAGENTA")
            || fnStartWith("Magenta")
            || fnStartWith("magenta")){
        return colorf::MAGENTA + fn_A_SHF();
    }

    for(const auto rf: {"0x%x", "0x%X", "0X%x", "0X%X", "%d"}){
        for(const auto gf: {"0x%x", "0x%X", "0X%x", "0X%X", "%d"}){
            for(const auto bf: {"0x%x", "0x%X", "0X%x", "0X%X", "%d"}){
                for(const auto af: {"0x%x", "0x%X", "0X%x", "0X%X", "%d"}){
                    int r = 0;
                    int g = 0;
                    int b = 0;
                    int a = 0;
                    if(false
                            || std::sscanf(colorString, str_printf("RGBA(%s,%s,%s,%s)", rf, gf, bf, af).c_str(), &r, &g, &b, &a) == 4
                            || std::sscanf(colorString, str_printf("Rgba(%s,%s,%s,%s)", rf, gf, bf, af).c_str(), &r, &g, &b, &a) == 4
                            || std::sscanf(colorString, str_printf("rgba(%s,%s,%s,%s)", rf, gf, bf, af).c_str(), &r, &g, &b, &a) == 4){
                        if(false
                                || r < 0
                                || g < 0
                                || b < 0
                                || a < 0){
                            throw fflerror("invalid color: %s", colorString);
                        }
                        else{
                            return RGBA(round255(r), round255(g), round255(b), round255(a));
                        }
                    }
                }
            }
        }
    }

    for(const auto rf: {"0x%x", "0x%X", "0X%x", "0X%X", "%d"}){
        for(const auto gf: {"0x%x", "0x%X", "0X%x", "0X%X", "%d"}){
            for(const auto bf: {"0x%x", "0x%X", "0X%x", "0X%X", "%d"}){
                int r = 0;
                int g = 0;
                int b = 0;
                if(false
                        || std::sscanf(colorString, str_printf("RGB(%s,%s,%s)", rf, gf, bf).c_str(), &r, &g, &b) == 3
                        || std::sscanf(colorString, str_printf("Rgb(%s,%s,%s)", rf, gf, bf).c_str(), &r, &g, &b) == 3
                        || std::sscanf(colorString, str_printf("rgb(%s,%s,%s)", rf, gf, bf).c_str(), &r, &g, &b) == 3){
                    if(false
                            || r < 0
                            || g < 0
                            || b < 0){
                        throw fflerror("invalid color: %s", colorString);
                    }
                    else{
                        return RGBA(round255(r), round255(g), round255(b), 255);
                    }
                }
            }
        }
    }

    throw fflerror("invalid color string: %s", colorString);
}
