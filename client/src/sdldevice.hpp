/*
 * =====================================================================================
 *
 *       Filename: sdldevice.hpp
 *        Created: 03/07/2016 23:57:04
 *  Last Modified: 03/11/2016 23:11:43
 *
 *    Description: copy from flare-engine:
 *				   SDLHardwareRenderDevice.h/cpp
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
#include <algorithm>

#include <SDL2/SDL.h>

#include "xmlext.hpp"

class SDLDevice final
{
    public:
        SDLDevice(const XMLExt &);
       ~SDLDevice();

    public:
       SDL_Texture *CreateTexture(const uint8_t *, size_t);

    public:
       void Present()
       {
           SDL_RenderPresent(m_Renderer);
       }

       void SetWindowTitle(const char *szUTF8Title)
       {
           SDL_SetWindowTitle(m_Window, (szUTF8Title) ? szUTF8Title : "");
       }

       void SetGamma(double fGamma)
       {
           Uint16 pRawRamp[256];
           SDL_CalculateGammaRamp((float)(std::min(std::max(fGamma, 0.0), 1.0)), pRawRamp);
           SDL_SetWindowGammaRamp(m_Window, pRawRamp, pRawRamp, pRawRamp);
       }

       void ClearScreen()
       {
           SetColor(0, 0, 0, 0XFF);
           SDL_RenderClear(m_Renderer);
       }

       void DrawLine(int nX, int nY, int nW, int nH)
       {
           SDL_RenderDrawLine(m_Renderer, nX, nY, nX + nW, nY + nH);
       }

       void SetColor(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
       {
           SDL_SetRenderDrawColor(m_Renderer, nR, nG, nB, nA);
       }

       void DrawPixel(int nX, int nY)
       {
           SDL_RenderDrawPoint(m_Renderer, nX, nY);
       }

       void DrawRectangle(int nX, int nY, int nW, int nH)
       {
           DrawLine(nX     , nY     , nX + nW, nY     );
           DrawLine(nX     , nY     , nX     , nY + nH);
           DrawLine(nX + nW, nY     , nX + nW, nY + nH);
           DrawLine(nX     , nY + nH, nX + nW, nY + nH);
       }

    private:
       // for graphics hardware
       SDL_Window   *m_Window;
       SDL_Renderer *m_Renderer;

    private:
       // for sound
};
