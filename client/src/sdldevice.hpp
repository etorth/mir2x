/*
 * =====================================================================================
 *
 *       Filename: sdldevice.hpp
 *        Created: 03/07/2016 23:57:04
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
#include <array>
#include <vector>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "colorfunc.hpp"

class SDLDevice final
{
    public:
        struct EnableDrawColor
        {
            EnableDrawColor(uint32_t);
            ~EnableDrawColor();
        };

        struct EnableDrawBlendMode
        {
            EnableDrawBlendMode(SDL_BlendMode);
            ~EnableDrawBlendMode();
        };

    private:
        using ColoStackNode      = std::array<uint32_t, 2>;
        using BlendModeStackNode = std::pair<SDL_BlendMode, uint32_t>;

    private:
       SDL_Window   *m_Window;
       SDL_Renderer *m_Renderer;

    private:
       std::vector<ColoStackNode>      m_ColorStack;
       std::vector<BlendModeStackNode> m_BlendModeStack;

    private:
       int m_WindowW;
       int m_WindowH;

    private:
       // for sound

    public:
        SDLDevice();
       ~SDLDevice();

    public:
       SDL_Texture *CreateTexture(const uint8_t *, size_t);

    public:
       void SetWindowIcon();
       void DrawTexture(SDL_Texture *, int, int);
       void DrawTexture(SDL_Texture *, int, int, int, int, int, int);
       void DrawTexture(SDL_Texture *, int, int, int, int, int, int, int, int);

    public:
       void DrawTextureEx(SDL_Texture *,  
               int,     // x on src
               int,     // y on src
               int,     // w on src
               int,     // h on src
               int,     // x on dst
               int,     // y on dst
               int,     // w on dst
               int,     // h on dst
               int,     // center x on dst
               int,     // center y on dst
               int);    // rotate in degree on dst

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
           SetColor(0, 0, 0, 0);
           SDL_RenderClear(m_Renderer);
       }

       void DrawLine(int nX0, int nY0, int nX1, int nY1)
       {
           SDL_RenderDrawLine(m_Renderer, nX0, nY0, nX1, nY1);
       }

       void SetColor(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
       {
           SDL_SetRenderDrawColor(m_Renderer, nR, nG, nB, nA);
       }

       void FillRectangle(int nX, int nY, int nW, int nH)
       {
           SDL_Rect stRect;
           stRect.x = nX;
           stRect.y = nY;
           stRect.w = nW;
           stRect.h = nH;

           SDL_RenderFillRect(m_Renderer, &stRect);
       }

       void FillRectangle(uint32_t, int nX, int nY, int nW, int nH)
       {
           SDL_Rect stRect;
           stRect.x = nX;
           stRect.y = nY;
           stRect.w = nW;
           stRect.h = nH;

           SDL_RenderFillRect(m_Renderer, &stRect);
       }

       SDL_Color Color()
       {
           return ColorFunc::RGBA2Color(m_ColorStack.back()[0]);
       }

       void DrawPixel(int nX, int nY)
       {
           SDL_RenderDrawPoint(m_Renderer, nX, nY);
       }

       void DrawRectangle(int nX, int nY, int nW, int nH)
       {
           SDL_Rect stRect;
           stRect.x = nX;
           stRect.y = nY;
           stRect.w = nW;
           stRect.h = nH;

           SDL_RenderDrawRect(m_Renderer, &stRect);
       }

       void DrawRectangle(int nFrameLineWidth, int nX, int nY, int nW, int nH)
       {
           if(nFrameLineWidth > 0){
               if(nFrameLineWidth == 1){
                   DrawRectangle(nX, nY, nW, nH);
               }else{
                   FillRectangle(nX, nY,                            nW, nFrameLineWidth);
                   FillRectangle(nX, nY + nW - 2 * nFrameLineWidth, nW, nFrameLineWidth);

                   FillRectangle(nX,                            nY + nFrameLineWidth, nFrameLineWidth, nH - 2 * nFrameLineWidth);
                   FillRectangle(nX + nW - 2 * nFrameLineWidth, nY + nFrameLineWidth, nFrameLineWidth, nH - 2 * nFrameLineWidth);
               }
           }
       }

       SDL_Texture *CreateTextureFromSurface(SDL_Surface * pSurface)
       {
           return pSurface ? SDL_CreateTextureFromSurface(m_Renderer, pSurface) : nullptr;
       }

       int WindowW(bool bRealWindowSizeInPixel)
       {
           int nW, nH;
           if(bRealWindowSizeInPixel){
               SDL_GetWindowSize(m_Window, &nW, &nH);
               return nW;
           }else{
               return m_WindowW;
           }
       }

       int WindowH(bool bRealWindowSizeInPixel)
       {
           int nW, nH;
           if(bRealWindowSizeInPixel){
               SDL_GetWindowSize(m_Window, &nW, &nH);
               return nH;
           }else{
               return m_WindowH;
           }
       }

    public:
       void PushColor(uint8_t, uint8_t, uint8_t, uint8_t);
       void PushColor(const SDL_Color &rstColor)
       {
           PushColor(rstColor.r, rstColor.g, rstColor.b, rstColor.a);
       }

    public:
       void PopColor();

    public:
       void PushBlendMode(SDL_BlendMode);
       void PopBlendMode();

    public:
       TTF_Font *CreateTTF(const uint8_t *, size_t, uint8_t);

    public:
       void CreateMainWindow();
       void CreateInitViewWindow();

    public:
       TTF_Font *DefaultTTF(uint8_t);
};
