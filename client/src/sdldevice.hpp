#pragma once
#include <array>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "totype.hpp"
#include "fflerror.hpp"
#include "fpsmonitor.hpp"
#include "soundeffecthandle.hpp"

class SDLDevice;
namespace SDLDeviceHelper
{
    class EnableRenderColor final
    {
        private:
            SDLDevice *m_device;

        private:
            Uint8 m_r;
            Uint8 m_g;
            Uint8 m_b;
            Uint8 m_a;

        public:
            /* ctor */  EnableRenderColor(uint32_t, SDLDevice * = nullptr);
            /* dtor */ ~EnableRenderColor();
    };

    struct EnableRenderBlendMode
    {
        private:
            SDLDevice *m_device;

        private:
            SDL_BlendMode m_blendMode;

        public:
            /* ctor */  EnableRenderBlendMode(SDL_BlendMode, SDLDevice * = nullptr);
            /* dtor */ ~EnableRenderBlendMode();
    };

    class RenderNewFrame final
    {
        private:
            SDLDevice *m_device;

        public:
            /* ctor */  RenderNewFrame(SDLDevice * = nullptr);
            /* dtor */ ~RenderNewFrame();
    };

    class EnableTextureBlendMode final
    {
        private:
            SDL_BlendMode m_blendMode;

        private:
            SDL_Texture *m_texPtr;

        public:
            /* ctor */  EnableTextureBlendMode(SDL_Texture *, SDL_BlendMode);
            /* dtor */ ~EnableTextureBlendMode();
    };

    class EnableTextureModColor final
    {
        private:
            Uint8 m_r;
            Uint8 m_g;
            Uint8 m_b;
            Uint8 m_a;

        private:
            SDL_Texture *m_texPtr;

        public:
            /* ctor */  EnableTextureModColor(SDL_Texture *, uint32_t);
            /* dtor */ ~EnableTextureModColor();
    };

    struct SDLEventPLoc final
    {
        const int x = -1;
        const int y = -1;

        operator bool () const
        {
            return x >= 0 && y >= 0;
        }
    };

    char getKeyChar(const SDL_Event &, bool);

    SDLEventPLoc getMousePLoc();
    SDLEventPLoc getEventPLoc(const SDL_Event &);

    std::tuple<int, int> getTextureSize(SDL_Texture *);
    int getTextureWidth (SDL_Texture *);
    int getTextureHeight(SDL_Texture *);
}

class SDLDevice final
{
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
       std::mutex m_freeChannelLock;
       std::vector<int> m_freeChannelList;
       std::unordered_map<int, std::shared_ptr<SoundEffectHandle>> m_busyChannelList;

    public:
       /* ctor */  SDLDevice();
       /* dtor */ ~SDLDevice();

    public:
       SDL_Texture *loadPNGTexture(const void *, size_t);

    public:
       void setWindowIcon();
       void toggleWindowFullscreen();

    public:
       void drawTexture(SDL_Texture *, int, int);
       void drawTexture(SDL_Texture *, int, int, int, int, int, int);
       void drawTexture(SDL_Texture *, int, int, int, int, int, int, int, int);

    public:
       void drawTextureExt(SDL_Texture *,
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
               int,     // rotate in 360-degree on dst
               SDL_RendererFlip = SDL_FLIP_NONE);

    public:
       void present()
       {
           SDL_RenderPresent(m_renderer);
       }

       void setWindowTitle(const char *szUTF8Title)
       {
           SDL_SetWindowTitle(m_window, (szUTF8Title) ? szUTF8Title : "");
       }

       void setGamma(double fGamma)
       {
           Uint16 pRawRamp[256];
           SDL_CalculateGammaRamp((float)((std::min<double>)((std::max<double>)(fGamma, 0.0), 1.0)), pRawRamp);
           SDL_SetWindowGammaRamp(m_window, pRawRamp, pRawRamp, pRawRamp);
       }

       void clearScreen()
       {
           setColor(0, 0, 0, 0);
           SDL_RenderClear(m_renderer);
       }

       void drawLine(int nX0, int nY0, int nX1, int nY1)
       {
           SDL_RenderDrawLine(m_renderer, nX0, nY0, nX1, nY1);
       }

       void drawLine(uint32_t color, int nX0, int nY0, int nX1, int nY1)
       {
           SDLDeviceHelper::EnableRenderColor enableColor(color, this);
           SDL_RenderDrawLine(m_renderer, nX0, nY0, nX1, nY1);
       }

       void drawCross(uint32_t color, int x, int y, size_t r)
       {
           SDLDeviceHelper::EnableRenderColor enableColor(color, this);
           SDL_RenderDrawLine(m_renderer, x - to_d(r), y, x + to_d(r), y);
           SDL_RenderDrawLine(m_renderer, x, y - to_d(r), x, y + to_d(r));
       }

       void setColor(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
       {
           SDL_SetRenderDrawColor(m_renderer, nR, nG, nB, nA);
       }

       void drawPixel(int nX, int nY)
       {
           SDL_RenderDrawPoint(m_renderer, nX, nY);
       }

    public:
       void fillRectangle(          int, int, int, int, int = 0);
       void fillRectangle(uint32_t, int, int, int, int, int = 0);

    public:
       void drawRectangle(          int, int, int, int, int = 0);
       void drawRectangle(uint32_t, int, int, int, int, int = 0);

    public:
       void drawWidthRectangle(          size_t, int, int, int, int);
       void drawWidthRectangle(uint32_t, size_t, int, int, int, int);

    public:
       void drawHLineFading(uint32_t, uint32_t, int, int, int);
       void drawVLineFading(uint32_t, uint32_t, int, int, int);

    public:
       void drawBoxFading(uint32_t, uint32_t, int, int, int, int, int, int);

    public:
       SDL_Renderer *getRenderer()
       {
           return m_renderer;
       }

    public:
       SDL_Texture *createTextureFromSurface(SDL_Surface * surfPtr)
       {
           return surfPtr ? SDL_CreateTextureFromSurface(m_renderer, surfPtr) : nullptr;
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
               throw fflerror("SDL_GetRendererOutputSize(%p) failed: %s", to_cvptr(m_renderer), SDL_GetError());
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
       TTF_Font *createTTF(const void *, size_t, uint8_t);

    public:
       void createMainWindow();
       void createInitViewWindow();

    public:
       SDL_Texture *createRGBATexture(const uint32_t *, size_t, size_t);

    public:
       TTF_Font *defaultTTF(uint8_t);

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
       SDL_Texture *getCover(int, int);

    public:
       void drawString(uint32_t, int, int, const char *);

    public:
       void stopBGM();
       void playBGM(Mix_Music *, int loops = -1);

    public:
       int playSoundEffect(std::shared_ptr<SoundEffectHandle>);

    private:
       static void recycleSoundEffectChannel(int);
};
