/*
 * =====================================================================================
 *
 *       Filename: colorfunc.hpp
 *        Created: 03/31/2016 19:46:27
 *  Last Modified: 06/19/2017 23:07:46
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
#pragma once
#include <cstdint>
#include <SDL2/SDL.h>

namespace ColorFunc
{
    const SDL_Color COLOR_WHITE = {0XFF, 0XFF, 0XFF, 0XFF};
    const SDL_Color COLOR_BLACK = {0X00, 0X00, 0X00, 0XFF};
    const SDL_Color COLOR_RED   = {0XFF, 0X00, 0X00, 0XFF};
    const SDL_Color COLOR_GREEN = {0X00, 0XFF, 0X00, 0XFF};
    const SDL_Color COLOR_BLUE  = {0X00, 0X00, 0XFF, 0XFF};

    SDL_Color MakeColor(uint8_t, uint8_t, uint8_t, uint8_t);

    uint32_t Color2U32RGBA(const SDL_Color &);
    uint32_t Color2U32ARGB(const SDL_Color &);

    SDL_Color U32RGBA2Color(uint32_t);
    SDL_Color U32ARGB2Color(uint32_t);

    bool String2Color(SDL_Color *, const char *);
}
