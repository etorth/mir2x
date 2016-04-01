/*
 * =====================================================================================
 *
 *       Filename: colorfunc.hpp
 *        Created: 03/31/2016 19:46:27
 *  Last Modified: 03/31/2016 20:03:23
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

uint32_t Color2U32RGBA(const SDL_Color &);
uint32_t Color2U32ARGB(const SDL_Color &);

SDL_Color U32RGBA2Color(uint32_t);
SDL_Color U32ARGB2Color(uint32_t);

bool String2Color(SDL_Color *, const char *);
