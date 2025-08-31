#pragma once
#include <array>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <mutex>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "totype.hpp"
#include "protocoldef.hpp"
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

    struct EnableRenderCropRectangle
    {
        private:
            SDLDevice *m_device;

        private:
            const bool m_clipped;

        private:
            SDL_Rect m_rect;

        public:
            /* ctor */  EnableRenderCropRectangle(int, int, int, int, SDLDevice * = nullptr);
            /* dtor */ ~EnableRenderCropRectangle();
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

    class EnableRenderTarget final
    {
        private:
            SDLDevice *m_device;

        private:
            SDL_Texture *m_target;

        public:
            /* ctor */  EnableRenderTarget(SDL_Texture *, SDLDevice * = nullptr);
            /* dtor */ ~EnableRenderTarget();
    };

    struct SDLEventPLoc final
    {
        const int x = 0;
        const int y = 0;
    };

    char getKeyChar(const SDL_Event &, bool);

    SDLEventPLoc getMousePLoc();
    std::tuple<int, int, Uint32> getMouseState();

    std::optional<SDLEventPLoc> getEventPLoc(const SDL_Event &);

    std::tuple<int, int> getTextureSize(SDL_Texture *);
    int getTextureWidth (SDL_Texture *);
    int getTextureHeight(SDL_Texture *);
}

class SDLDevice;
class SDLSoundEffectChannel // controller of sound effect and channl playing it
{
    private:
        friend class SDLDevice;

    private:
        SDLDevice *m_sdlDevice;

    private:
        int m_channel;

    private:
        SDLSoundEffectChannel(SDLDevice *, int);

    private:
        SDLSoundEffectChannel              (      SDLSoundEffectChannel &&) = delete;
        SDLSoundEffectChannel              (const SDLSoundEffectChannel  &) = delete;
        SDLSoundEffectChannel & operator = (      SDLSoundEffectChannel &&) = delete;
        SDLSoundEffectChannel & operator = (const SDLSoundEffectChannel  &) = delete;

    public:
        virtual ~SDLSoundEffectChannel();

    public:
        // returns true if this->halt() get called
        // channel may have stopped before this->halt() because of finished playing or errors
        bool halted() const
        {
            return m_channel < 0;
        }

    public:
        void halt();
        void pause();
        void resume();
        void setPosition(int, int);
};

class SDLDevice final
{
    private:
        friend class SDLSoundEffectChannel;

    private:
        struct SoundChannelHookState
        {
            bool hooked = false;
            std::shared_ptr<SoundEffectHandle> handle {};
        };

    private:
        const size_t m_channelCount = 128;

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
       std::unordered_set<int> m_freeChannelList;
       std::unordered_map<int, SoundChannelHookState> m_channelStateList;

    public:
       /* ctor */  SDLDevice();
       /* dtor */ ~SDLDevice();

    public:
       SDL_Texture *loadPNGTexture(const void *, size_t);

    public:
       void setWindowIcon();
       void toggleWindowFullscreen();

    public:
       void drawTexture(SDL_Texture *, dir8_t, int, int);

    public:
       void drawTexture(SDL_Texture *, int, int);
       void drawTexture(SDL_Texture *, int, int, int, int, int, int);
       void drawTexture(SDL_Texture *, int, int, int, int, int, int, int, int);

    public:
       void drawTextureEx(SDL_Texture *,
               int, int,
               int, int, // src region

               int, int,
               int, int, // dst region

               int, // center x on dst
               int, // center y on dst

               int, // rotate in 360-degree on dst
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

       void drawLine(int argX0, int argY0, int argX1, int argY1)
       {
           SDL_RenderDrawLine(m_renderer, argX0, argY0, argX1, argY1);
       }

       void drawLine(uint32_t color, int argX0, int argY0, int argX1, int argY1)
       {
           SDLDeviceHelper::EnableRenderColor enableColor(color, this);
           SDL_RenderDrawLine(m_renderer, argX0, argY0, argX1, argY1);
       }

       void drawCross(uint32_t color, int x, int y, size_t r)
       {
           SDLDeviceHelper::EnableRenderColor enableColor(color, this);
           SDL_RenderDrawLine(m_renderer, x - to_d(r), y, x + to_d(r), y);
           SDL_RenderDrawLine(m_renderer, x, y - to_d(r), x, y + to_d(r));
       }

       void drawLinef(float argX0, float argY0, float argX1, float argY1)
       {
           SDL_RenderDrawLine(m_renderer, argX0, argY0, argX1, argY1);
       }

       void drawLinef(uint32_t color, float argX0, float argY0, float argX1, float argY1)
       {
           SDLDeviceHelper::EnableRenderColor enableColor(color, this);
           SDL_RenderDrawLine(m_renderer, argX0, argY0, argX1, argY1);
       }

       void drawCrossf(uint32_t color, float x, float y, float r)
       {
           SDLDeviceHelper::EnableRenderColor enableColor(color, this);
           SDL_RenderDrawLine(m_renderer, x - r, y, x + r, y);
           SDL_RenderDrawLine(m_renderer, x, y - r, x, y + r);
       }

       void setColor(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
       {
           SDL_SetRenderDrawColor(m_renderer, nR, nG, nB, nA);
       }

       void drawPixel(int argX, int argY)
       {
           SDL_RenderDrawPoint(m_renderer, argX, argY);
       }

    public:
       void drawTriangle(          int, int, int, int, int, int);
       void drawTriangle(uint32_t, int, int, int, int, int, int);

       void fillTriangle(          int, int, int, int, int, int);
       void fillTriangle(uint32_t, int, int, int, int, int, int);

    public:
       void drawRectangle(          int, int, int, int, int = 0);
       void drawRectangle(uint32_t, int, int, int, int, int = 0);

       void fillRectangle(          int, int, int, int, int = 0);
       void fillRectangle(uint32_t, int, int, int, int, int = 0);

    public:
       void drawCircle(uint32_t, int, int, int);
       void fillCircle(uint32_t, int, int, int);

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
       SDL_Texture *createTargetTexture(size_t, size_t);
       SDL_Texture *createRGBATexture(const uint32_t *, size_t, size_t);

    public:
       void destroyTexture(SDL_Texture *);

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
       SDL_Texture *getCover(int, int); // diameter = 2 * r - 1, r >= 1

    public:
       void drawString(uint32_t, int, int, const char *);

    public:
       void setWindowSize(int, int);

    public:
       void stopBGM();
       void setBGMVolume(float); // by initial max volume
       void playBGM(Mix_Music *, size_t repeats = 0); // by default repeats forever

    public:
       size_t channelCount() const
       {
           return m_channelCount;
       }

       // repeats : 0 : plays forever
       //           N : repeat N times
       //
       // distance:    0 : overlaps with listener
       //            255 : far enough but may not be competely silent
       //          > 255 : culled, will not play
       //
       // angle:   0 : north
       //         90 : east
       //        180 : south
       //        270 : west
       //
       // return   empty : no channel allocated for playing
       //      non-empty : playing channel, channel can not get reused before halt() or dtor() called
       std::shared_ptr<SDLSoundEffectChannel> playSoundEffect(std::shared_ptr<SoundEffectHandle>, int distance = 0, int angle = 0, size_t repeats = 1);
       void stopSoundEffect();
       void setSoundEffectVolume(float);

    private:
       static void recycleSoundEffectChannel(int);
};
