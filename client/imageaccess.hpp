/*
 * =====================================================================================
 *
 *       Filename: imageaccess.hpp
 *        Created: 01/14/2016 00:57:45
 *  Last Modified: 01/14/2016 01:00:47
 *
 *    Description: load any file format into a SDL_Texture
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
#include <SDL.h>

SDL_Texture *LoadSDLTextureFromFile(const char *, SDL_Renderer *);

