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
#include <map>
#include <array>
#include <vector>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "colorf.hpp"
#include "fflerror.hpp"

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
        struct ColorStackNode
        {
            uint32_t Color;
            size_t   Repeat;

            ColorStackNode(uint32_t nColor, size_t nRepeat)
                : Color(nColor)
                , Repeat(nRepeat)
            {}
        };

        struct BlendModeStackNode
        {
            SDL_BlendMode BlendMode;
            size_t        Repeat;

            BlendModeStackNode(SDL_BlendMode stBlendMode, size_t nRepeat)
                : BlendMode(stBlendMode)
                , Repeat(nRepeat)
            {}
        };

    private:
       SDL_Window   *m_window;
       SDL_Renderer *m_renderer;

    private:
       std::vector<ColorStackNode>     m_colorStack;
       std::vector<BlendModeStackNode> m_blendModeStack;

    private:
       std::map<uint8_t, TTF_Font *> m_innFontMap;

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
       void drawTextureEx(SDL_Texture *,  
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
       void present()
       {
           SDL_RenderPresent(m_renderer);
       }

       void SetWindowTitle(const char *szUTF8Title)
       {
           SDL_SetWindowTitle(m_window, (szUTF8Title) ? szUTF8Title : "");
       }

       void SetGamma(double fGamma)
       {
           Uint16 pRawRamp[256];
           SDL_CalculateGammaRamp((float)((std::min<double>)((std::max<double>)(fGamma, 0.0), 1.0)), pRawRamp);
           SDL_SetWindowGammaRamp(m_window, pRawRamp, pRawRamp, pRawRamp);
       }

       void clearScreen()
       {
           SetColor(0, 0, 0, 0);
           SDL_RenderClear(m_renderer);
       }

       void DrawLine(int nX0, int nY0, int nX1, int nY1)
       {
           SDL_RenderDrawLine(m_renderer, nX0, nY0, nX1, nY1);
       }

       void SetColor(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
       {
           SDL_SetRenderDrawColor(m_renderer, nR, nG, nB, nA);
       }

       void fillRectangle(int nX, int nY, int nW, int nH)
       {
           SDL_Rect stRect;
           stRect.x = nX;
           stRect.y = nY;
           stRect.w = nW;
           stRect.h = nH;

           SDL_RenderFillRect(m_renderer, &stRect);
       }

       void fillRectangle(uint32_t nRGBA, int nX, int nY, int nW, int nH)
       {
           EnableDrawColor stEnableColor(nRGBA);

           SDL_Rect stRect;
           stRect.x = nX;
           stRect.y = nY;
           stRect.w = nW;
           stRect.h = nH;

           SDL_RenderFillRect(m_renderer, &stRect);
       }

       void DrawPixel(int nX, int nY)
       {
           SDL_RenderDrawPoint(m_renderer, nX, nY);
       }

       void DrawRectangle(int nX, int nY, int nW, int nH)
       {
           SDL_Rect stRect;
           stRect.x = nX;
           stRect.y = nY;
           stRect.w = nW;
           stRect.h = nH;

           SDL_RenderDrawRect(m_renderer, &stRect);
       }

       void DrawRectangle(int nFrameLineWidth, int nX, int nY, int nW, int nH)
       {
           if(nFrameLineWidth > 0){
               if(nFrameLineWidth == 1){
                   DrawRectangle(nX, nY, nW, nH);
               }else{
                   fillRectangle(nX, nY,                            nW, nFrameLineWidth);
                   fillRectangle(nX, nY + nW - 2 * nFrameLineWidth, nW, nFrameLineWidth);

                   fillRectangle(nX,                            nY + nFrameLineWidth, nFrameLineWidth, nH - 2 * nFrameLineWidth);
                   fillRectangle(nX + nW - 2 * nFrameLineWidth, nY + nFrameLineWidth, nFrameLineWidth, nH - 2 * nFrameLineWidth);
               }
           }
       }

       SDL_Texture *CreateTextureFromSurface(SDL_Surface * pSurface)
       {
           return pSurface ? SDL_CreateTextureFromSurface(m_renderer, pSurface) : nullptr;
       }

       int WindowW(bool bRealWindowSizeInPixel)
       {
           if(bRealWindowSizeInPixel){
               return std::get<0>(getWindowSize());
           }else{
               return std::get<0>(getRendererSize());
           }
       }

       int WindowH(bool bRealWindowSizeInPixel)
       {
           if(bRealWindowSizeInPixel){
               return std::get<1>(getWindowSize());
           }else{
               return std::get<1>(getRendererSize());
           }
       }

       std::tuple<int, int> getWindowSize()
       {
           int w = -1;
           int h = -1;

           SDL_GetWindowSize(m_window, &w, &h);
           return {w, h};
       }

       std::tuple<int, int> getRendererSize()
       {
           int w = -1;
           int h = -1;

           if(SDL_GetRendererOutputSize(m_renderer, &w, &h)){
               throw fflerror("SDL_GetRendererOutputSize(%p) failed", m_renderer);
           }
           return {w, h};
       }

    public:
       void PushColor(uint8_t, uint8_t, uint8_t, uint8_t);

    public:
       void PushColor(uint32_t nRGBA)
       {
           PushColor(colorf::R(nRGBA), colorf::G(nRGBA), colorf::B(nRGBA), colorf::A(nRGBA));
       }

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
       SDL_Texture *createTexture(const uint32_t *, int, int);

    public:
       TTF_Font *DefaultTTF(uint8_t);

    public:
       void setWindowResizable(bool resizable)
       {
           SDL_SetWindowResizable(m_window, resizable ? SDL_TRUE : SDL_FALSE);
       }

    public:
       static std::tuple<int, int> getTextureSize(SDL_Texture *texture)
       {
           if(!texture){
               throw fflerror("null texture");
           }

           int width  = 0;
           int height = 0;

           if(!SDL_QueryTexture(const_cast<SDL_Texture *>(texture), 0, 0, &width, &height)){
               return {width, height};
           }

           throw fflerror("query texture failed: %p", texture);
       }

       static int getTextureWidth(SDL_Texture *texture)
       {
           return std::get<0>(getTextureSize(texture));
       }

       static int getTextureHeight(SDL_Texture *texture)
       {
           return std::get<1>(getTextureSize(texture));
       }
};
