#pragma once
#include <array>
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_gfxPrimitives.h>
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

    std::tuple<int, int> getTextureSize  (const SDL_Texture *);
    int                  getTextureWidth (const SDL_Texture *, std::optional<int> = std::nullopt);
    int                  getTextureHeight(const SDL_Texture *, std::optional<int> = std::nullopt);
}

class SDLDevice;
class SDLSoundEffectChannel // controller of sound effect and the track playing it
{
    private:
        friend class SDLDevice;

    private:
        SDLDevice *m_sdlDevice;

    private:
        MIX_Track *m_track;       // nullptr after halt(); owned by m_sdlDevice's track pool

    private:
        SDLSoundEffectChannel(SDLDevice *, MIX_Track *);

    private:
        SDLSoundEffectChannel              (      SDLSoundEffectChannel &&) = delete;
        SDLSoundEffectChannel              (const SDLSoundEffectChannel  &) = delete;
        SDLSoundEffectChannel & operator = (      SDLSoundEffectChannel &&) = delete;
        SDLSoundEffectChannel & operator = (const SDLSoundEffectChannel  &) = delete;

    public:
        virtual ~SDLSoundEffectChannel();

    public:
        // returns true if this->halt() get called
        // track may have stopped before this->halt() because of finished playing or errors
        bool halted() const
        {
            return m_track == nullptr;
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
        const size_t m_trackCount = 128;

    private:
        // Declaration order matters: SDL requires the renderer be destroyed before its owning window
        // So m_renderer must be declared AFTER m_window
        std::unique_ptr<SDL_Window,   void(*)(SDL_Window   *)> m_window   {nullptr, SDL_DestroyWindow  };
        std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer *)> m_renderer {nullptr, SDL_DestroyRenderer};

    private:
       FPSMonitor m_fpsMonitor;

    private:
       std::unordered_map<int, SDL_Texture *> m_cover;

    private:
       std::unordered_map<uint8_t, TTF_Font *> m_fontList;

    private:
       MIX_Mixer *m_mixer = nullptr;
       MIX_Track *m_bgmTrack = nullptr;
       std::vector<MIX_Track *> m_tracks;          // all sound-effect tracks, owned here

    private:
       std::mutex m_freeTrackLock;
       std::unordered_set<MIX_Track *> m_freeTrackList;
       std::unordered_map<MIX_Track *, SoundChannelHookState> m_trackStateList;

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
               SDL_FlipMode = SDL_FLIP_NONE);

    public:
       void present()
       {
           SDL_RenderPresent(m_renderer.get());
       }

       void setWindowTitle(const char *szUTF8Title)
       {
           SDL_SetWindowTitle(m_window.get(), (szUTF8Title) ? szUTF8Title : "");
       }

       void clearScreen()
       {
           setColor(0, 0, 0, 0);
           SDL_RenderClear(m_renderer.get());
       }

       void drawLine(int argX0, int argY0, int argX1, int argY1)
       {
           SDL_RenderLine(m_renderer.get(), to_f(argX0), to_f(argY0), to_f(argX1), to_f(argY1));
       }

       void drawLine(uint32_t color, int argX0, int argY0, int argX1, int argY1)
       {
           SDLDeviceHelper::EnableRenderColor enableColor(color, this);
           drawLine(argX0, argY0, argX1, argY1);
       }

       void drawCross(uint32_t color, int x, int y, size_t r)
       {
           SDLDeviceHelper::EnableRenderColor enableColor(color, this);
           drawLine(x - to_d(r), y, x + to_d(r), y);
           drawLine(x, y - to_d(r), x, y + to_d(r));
       }

       void drawLinef(float argX0, float argY0, float argX1, float argY1)
       {
           SDL_RenderLine(m_renderer.get(), argX0, argY0, argX1, argY1);
       }

       void drawLinef(uint32_t color, float argX0, float argY0, float argX1, float argY1)
       {
           SDLDeviceHelper::EnableRenderColor enableColor(color, this);
           SDL_RenderLine(m_renderer.get(), argX0, argY0, argX1, argY1);
       }

       void drawCrossf(uint32_t color, float x, float y, float r)
       {
           SDLDeviceHelper::EnableRenderColor enableColor(color, this);
           SDL_RenderLine(m_renderer.get(), x - r, y, x + r, y);
           SDL_RenderLine(m_renderer.get(), x, y - r, x, y + r);
       }

       void setColor(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
       {
           SDL_SetRenderDrawColor(m_renderer.get(), nR, nG, nB, nA);
       }

       void drawPixel(int argX, int argY)
       {
           SDL_RenderPoint(m_renderer.get(), to_f(argX), to_f(argY));
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
           return m_renderer.get();
       }

    public:
       MIX_Mixer *getMixer()
       {
           return m_mixer;
       }

    public:
       SDL_Texture *createTextureFromSurface(SDL_Surface * surfPtr)
       {
           return surfPtr ? SDL_CreateTextureFromSurface(m_renderer.get(), surfPtr) : nullptr;
       }

       std::tuple<int, int> getWindowSize()
       {
           int w = -1;
           int h = -1;

           SDL_GetWindowSize(m_window.get(), &w, &h);
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

           if(!SDL_GetCurrentRenderOutputSize(m_renderer.get(), &w, &h)){
               throw fflpanic("SDL_GetCurrentRenderOutputSize({:p}) failed: {}", to_cvptr(m_renderer.get()), SDL_GetError());
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
           SDL_SetWindowResizable(m_window.get(), resizable);
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
       void playBGM(MIX_Audio *, size_t repeats = 0); // by default repeats forever

    public:
       size_t channelCount() const
       {
           return m_trackCount;
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
       // return   empty : no track allocated for playing
       //      non-empty : playing track, track can not get reused before halt() or dtor() called
       std::shared_ptr<SDLSoundEffectChannel> playSoundEffect(std::shared_ptr<SoundEffectHandle>, int distance = 0, int angle = 0, size_t repeats = 1);
       void stopSoundEffect();
       void setSoundEffectVolume(float);

    private:
       static void recycleSoundEffectTrack(void *userdata, MIX_Track *track);
       static void apply3DPosition(MIX_Track *track, int distance, int angle);
};
