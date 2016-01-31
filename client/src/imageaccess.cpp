/*
 * =====================================================================================
 *
 *       Filename: imageaccess.cpp
 *        Created: 01/14/2016 00:58:31
 *  Last Modified: 01/14/2016 01:00:46
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

#include "imageaccess.hpp"

SDL_Texture *LoadSDLTextureFromFile(const char * szFileFullName, SDL_Renderer * pRenderer)
{
    // we always assume SDL are fully initalized
    SDL_Surface *pSurface = IMG_Load(szFileFullName);
    SDL_Texture *pTexture = nullptr;
    if(pSurface && pRenderer){
        pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
        SDL_FreeSurface(pSurface);
    }
    return pTexture;
}
