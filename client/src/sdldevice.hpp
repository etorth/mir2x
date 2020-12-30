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
#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "totype.hpp"
#include "colorf.hpp"
#include "fflerror.hpp"
#include "fpsmonitor.hpp"

class SDLDevice final
{
    public:
        struct EventLocation
        {
            int  x = -1;
            int  y = -1;
            bool hasLocation = false;

            operator bool () const
            {
                return hasLocation;
            }
        };

    public:
        class EnableRenderColor
        {
            private:
                Uint8 m_r;
                Uint8 m_g;
                Uint8 m_b;
                Uint8 m_a;

            public:
                EnableRenderColor(uint32_t);
               ~EnableRenderColor();
        };

        struct EnableRenderBlendMode
        {
            private:
                SDL_BlendMode m_blendMode;

            public:
                EnableRenderBlendMode(SDL_BlendMode);
               ~EnableRenderBlendMode();
        };

        struct RenderNewFrame
        {
            RenderNewFrame();
           ~RenderNewFrame();
        };

    public:
        class EnableTextureBlendMode
        {
            private:
                SDL_BlendMode m_blendMode;

            private:
                SDL_Texture *m_texPtr;

            public:
                EnableTextureBlendMode(SDL_Texture *, SDL_BlendMode);
               ~EnableTextureBlendMode();
        };

    public:
        class EnableTextureModColor
        {
            private:
                Uint8 m_r;
                Uint8 m_g;
                Uint8 m_b;
                Uint8 m_a;

            private:
                SDL_Texture *m_texPtr;

            public:
                EnableTextureModColor(SDL_Texture *, uint32_t);
               ~EnableTextureModColor();
        };

    private:
       SDL_Window   *m_window   = nullptr;
       SDL_Renderer *m_renderer = nullptr;

    private:
       FPSMonitor m_fpsMonitor;

    private:
       std::unordered_map<int, SDL_Texture *> m_cover;

    private:
       std::unordered_map<uint8_t, TTF_Font *> m_fontList;

    private:
       // for sound

    public:
        SDLDevice();
       ~SDLDevice();

    public:
       SDL_Texture *CreateTexture(const uint8_t *, size_t);

    public:
       void SetWindowIcon();
       void drawTexture(SDL_Texture *, int, int);
       void drawTexture(SDL_Texture *, int, int, int, int, int, int);
       void drawTexture(SDL_Texture *, int, int, int, int, int, int, int, int);

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

       void drawLine(int nX0, int nY0, int nX1, int nY1)
       {
           SDL_RenderDrawLine(m_renderer, nX0, nY0, nX1, nY1);
       }

       void drawLine(uint32_t color, int nX0, int nY0, int nX1, int nY1)
       {
           EnableRenderColor enableColor(color);
           SDL_RenderDrawLine(m_renderer, nX0, nY0, nX1, nY1);
       }

       void SetColor(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
       {
           SDL_SetRenderDrawColor(m_renderer, nR, nG, nB, nA);
       }

       void DrawPixel(int nX, int nY)
       {
           SDL_RenderDrawPoint(m_renderer, nX, nY);
       }

    public:
       void fillRectangle(          int, int, int, int);
       void fillRectangle(uint32_t, int, int, int, int);

    public:
       void drawRectangle(          int, int, int, int);
       void drawRectangle(uint32_t, int, int, int, int);

    public:
       void drawWidthRectangle(          size_t, int, int, int, int);
       void drawWidthRectangle(uint32_t, size_t, int, int, int, int);

    public:
       SDL_Renderer *getRenderer()
       {
           return m_renderer;
       }

    public:
       SDL_Texture *CreateTextureFromSurface(SDL_Surface * pSurface)
       {
           return pSurface ? SDL_CreateTextureFromSurface(m_renderer, pSurface) : nullptr;
       }

       std::tuple<int, int> getWindowSize()
       {
           int w = -1;
           int h = -1;

           SDL_GetWindowSize(m_window, &w, &h);
           return {w, h};
       }

       int getWindowWidth()
       {
           return std::get<0>(getWindowSize());
       }

       int getWindowHeight()
       {
           return std::get<1>(getWindowSize());
       }

       std::tuple<int, int> getRendererSize()
       {
           int w = -1;
           int h = -1;

           if(SDL_GetRendererOutputSize(m_renderer, &w, &h)){
               throw fflerror("SDL_GetRendererOutputSize(%p) failed", to_cvptr(m_renderer));
           }
           return {w, h};
       }

       int getRendererWidth()
       {
           return std::get<0>(getRendererSize());
       }

       int getRendererHeight()
       {
           return std::get<1>(getRendererSize());
       }

    public:
       TTF_Font *createTTF(const uint8_t *, size_t, uint8_t);

    public:
       void CreateMainWindow();
       void CreateInitViewWindow();

    public:
       SDL_Texture *createTexture(const uint32_t *, int, int);

    public:
       TTF_Font *DefaultTTF(uint8_t);

    public:
       void updateFPS()
       {
           m_fpsMonitor.update();
       }

       size_t getFPS() const
       {
           return m_fpsMonitor.fps();
       }

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

           throw fflerror("query texture failed: %p", to_cvptr(texture));
       }

       static int getTextureWidth(SDL_Texture *texture)
       {
           return std::get<0>(getTextureSize(texture));
       }

       static int getTextureHeight(SDL_Texture *texture)
       {
           return std::get<1>(getTextureSize(texture));
       }

    public:
       SDL_Texture *getCover(int);

    public:
       static EventLocation getEventLocation(const SDL_Event &event)
       {
           switch(event.type){
               case SDL_MOUSEMOTION:
                   {
                       return {event.motion.x, event.motion.y, true};
                   }
               case SDL_MOUSEBUTTONUP:
               case SDL_MOUSEBUTTONDOWN:
                   {
                       return {event.button.x, event.button.y, true};
                   }
               default:
                   {
                       return {-1, -1, false};
                   }
           }
       }
};
