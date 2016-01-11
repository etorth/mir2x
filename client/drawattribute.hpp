/*
 * =====================================================================================
 *
 *       Filename: drawattribute.hpp
 *        Created: 6/29/2015 11:01:03 PM
 *  Last Modified: 06/29/2015 11:21:34 PM
 *
 *    Description: provide a DrawAttribuate to Device
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

#include <SDL.h>
#include <cstdint>

typedef struct{
    SDL_Texture *Texture;
    uint32_t     Method;
    int          SrcX;
    int          SrcY;
    int          SrcW;
    int          SrcH;
    int          DstX;
    int          DstY;
    int          DstW;
    int          DstH;
}DRAWATTRIBUTE;

DRAWATTRIBUTE CreateDrawAttribute(SDL_Texture *, int, int);
DRAWATTRIBUTE CreateDrawAttribute(SDL_Texture *, uint32_t, int, int);
