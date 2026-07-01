#include <cmath>
#include <ranges>
#include <numeric>
#include <inplace_vector>
#include <cinttypes>
#include <system_error>
#include <unordered_map>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include "log.hpp"
#include "mathf.hpp"
#include "totype.hpp"
#include "rawbuf.hpp"
#include "colorf.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "sdldevice.hpp"
#include "clientargparser.hpp"
#include "soundeffecthandle.hpp"

extern Log *g_mir2xLog;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

SDLDeviceHelper::EnableRenderColor::EnableRenderColor(uint32_t color, SDLDevice *devPtr)
    : m_device(devPtr ? devPtr : g_sdlDevice)
{
    if(!SDL_GetRenderDrawColor(m_device->getRenderer(), &m_r, &m_g, &m_b, &m_a)){
        throw fflpanic("get renderer draw color failed: {}", SDL_GetError());
    }

    if(!SDL_SetRenderDrawColor(m_device->getRenderer(), colorf::R(color), colorf::G(color), colorf::B(color), colorf::A(color))){
        throw fflpanic("set renderer draw color failed: {}", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderColor::~EnableRenderColor()
{
    if(!SDL_SetRenderDrawColor(m_device->getRenderer(), m_r, m_g, m_b, m_a)){
        g_mir2xLog->addLog(LOGTYPE_WARNING, "Set renderer draw color failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderBlendMode::EnableRenderBlendMode(SDL_BlendMode blendMode, SDLDevice *devPtr)
    : m_device(devPtr ? devPtr : g_sdlDevice)
{
    if(!SDL_GetRenderDrawBlendMode(m_device->getRenderer(), &m_blendMode)){
        throw fflpanic("get renderer blend mode failed: {}", SDL_GetError());
    }

    if(!SDL_SetRenderDrawBlendMode(m_device->getRenderer(), blendMode)){
        throw fflpanic("set renderer blend mode failed: {}", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderBlendMode::~EnableRenderBlendMode()
{
    if(!SDL_SetRenderDrawBlendMode(m_device->getRenderer(), m_blendMode)){
        g_mir2xLog->addLog(LOGTYPE_WARNING, "set renderer blend mode failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderCropRectangle::EnableRenderCropRectangle(int x, int y, int w, int h, SDLDevice *devPtr)
    : m_device(devPtr ? devPtr : g_sdlDevice)
    , m_clipped(SDL_RenderClipEnabled(m_device->getRenderer()))
{
    if(m_clipped){
        SDL_GetRenderClipRect(m_device->getRenderer(), &m_rect);
    }

    SDL_Rect newRect;
    newRect.x = x;
    newRect.y = y;
    newRect.w = w;
    newRect.h = h;

    if(!SDL_SetRenderClipRect(m_device->getRenderer(), &newRect)){
        throw fflpanic("set renderer clip rectangle failed: {}", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderCropRectangle::~EnableRenderCropRectangle()
{
    if(!SDL_SetRenderClipRect(m_device->getRenderer(), m_clipped ? &m_rect : nullptr)){
        g_mir2xLog->addLog(LOGTYPE_WARNING, "set renderer clip rectangle failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::RenderNewFrame::RenderNewFrame(SDLDevice *devPtr)
    : m_device(devPtr ? devPtr : g_sdlDevice)
{
    m_device->clearScreen();
}

SDLDeviceHelper::RenderNewFrame::~RenderNewFrame()
{
    m_device->updateFPS();
    m_device->present();
}

SDLDeviceHelper::EnableTextureBlendMode::EnableTextureBlendMode(SDL_Texture *texPtr, SDL_BlendMode mode)
    : m_texPtr(texPtr)
{
    if(!m_texPtr){
        return;
    }

    if(!SDL_GetTextureBlendMode(m_texPtr, &m_blendMode)){
        throw fflpanic("get texture blend mode failed: {}", SDL_GetError());
    }

    if(!SDL_SetTextureBlendMode(m_texPtr, mode)){
        throw fflpanic("set texture blend mode failed: {}", SDL_GetError());
    }
}

SDLDeviceHelper::EnableTextureBlendMode::~EnableTextureBlendMode()
{
    if(!m_texPtr){
        return;
    }

    if(!SDL_SetTextureBlendMode(m_texPtr, m_blendMode)){
        g_mir2xLog->addLog(LOGTYPE_WARNING, "Setup texture blend mode failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableTextureModColor::EnableTextureModColor(SDL_Texture *texPtr, uint32_t color)
    : m_texPtr(texPtr)
{
    if(!m_texPtr){
        return;
    }

    if(!SDL_GetTextureColorMod(m_texPtr, &m_r, &m_g, &m_b)){
        throw fflpanic("SDL_GetTextureColorMod({:p}) failed: {}", to_cvptr(m_texPtr), SDL_GetError());
    }

    if(!SDL_GetTextureAlphaMod(m_texPtr, &m_a)){
        throw fflpanic("SDL_GetTextureAlphaMod({:p}) failed: {}", to_cvptr(m_texPtr), SDL_GetError());
    }

    if(!SDL_SetTextureColorMod(m_texPtr, colorf::R(color), colorf::G(color), colorf::B(color))){
        throw fflpanic("SDL_SetTextureColorMod({:p}) failed: {}", to_cvptr(m_texPtr), SDL_GetError());
    }

    if(!SDL_SetTextureAlphaMod(m_texPtr, colorf::A(color))){
        throw fflpanic("SDL_SetTextureAlphaMod({:p}) failed: {}", to_cvptr(m_texPtr), SDL_GetError());
    }
}

SDLDeviceHelper::EnableTextureModColor::~EnableTextureModColor()
{
    if(!m_texPtr){
        return;
    }

    if(!SDL_SetTextureColorMod(m_texPtr, m_r, m_g, m_b)){
        g_mir2xLog->addLog(LOGTYPE_WARNING, "set texture mod color failed: %s", SDL_GetError());
    }

    if(!SDL_SetTextureAlphaMod(m_texPtr, m_a)){
        g_mir2xLog->addLog(LOGTYPE_WARNING, "set texture mod alpha failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderTarget::EnableRenderTarget(SDL_Texture *argTarget, SDLDevice * argDevice)
    : m_device(argDevice ? argDevice : g_sdlDevice)
    , m_target(SDL_GetRenderTarget(m_device->getRenderer()))
{
    if(argTarget){
        // SDL3 exposes texture properties for access mode (SDL_QueryTexture removed)
        const auto props = SDL_GetTextureProperties(argTarget);
        const auto access = static_cast<SDL_TextureAccess>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_ACCESS_NUMBER, -1));
        if(access != SDL_TEXTUREACCESS_TARGET){
            throw fflpanic("Texture can not be used as renderer target: {:p}", to_cvptr(argTarget));
        }
    }

    if(!SDL_SetRenderTarget(m_device->getRenderer(), argTarget)){
        throw fflpanic("SDL_SetRenderTarget({:p}) failed: {}", to_cvptr(argTarget), SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderTarget::~EnableRenderTarget()
{
    if(!SDL_SetRenderTarget(m_device->getRenderer(), m_target)){
        g_mir2xLog->addLog(LOGTYPE_WARNING, "SDL_SetRenderTarget(%p) failed: %s", to_cvptr(m_target), SDL_GetError());
    }
}

char SDLDeviceHelper::getKeyChar(const SDL_Event &event, bool checkShiftKey)
{
    const static std::unordered_map<SDL_Keycode, const char *> s_lookupTable
    {
        {SDLK_SPACE,        " "   " " },
        {SDLK_APOSTROPHE,   "'"   "\""},
        {SDLK_COMMA,        ","   "<" },
        {SDLK_MINUS,        "-"   "_" },
        {SDLK_PERIOD,       "."   ">" },
        {SDLK_SLASH,        "/"   "?" },
        {SDLK_0,            "0"   ")" },
        {SDLK_1,            "1"   "!" },
        {SDLK_2,            "2"   "@" },
        {SDLK_3,            "3"   "#" },
        {SDLK_4,            "4"   "$" },
        {SDLK_5,            "5"   "%" },
        {SDLK_6,            "6"   "^" },
        {SDLK_7,            "7"   "&" },
        {SDLK_8,            "8"   "*" },
        {SDLK_9,            "9"   "(" },
        {SDLK_SEMICOLON,    ";"   ":" },
        {SDLK_EQUALS,       "="   "+" },
        {SDLK_LEFTBRACKET,  "["   "{" },
        {SDLK_BACKSLASH,    "\\"  "|" },
        {SDLK_RIGHTBRACKET, "]"   "}" },
        {SDLK_GRAVE,        "`"   "~" },
        {SDLK_A,            "a"   "A" },
        {SDLK_B,            "b"   "B" },
        {SDLK_C,            "c"   "C" },
        {SDLK_D,            "d"   "D" },
        {SDLK_E,            "e"   "E" },
        {SDLK_F,            "f"   "F" },
        {SDLK_G,            "g"   "G" },
        {SDLK_H,            "h"   "H" },
        {SDLK_I,            "i"   "I" },
        {SDLK_J,            "j"   "J" },
        {SDLK_K,            "k"   "K" },
        {SDLK_L,            "l"   "L" },
        {SDLK_M,            "m"   "M" },
        {SDLK_N,            "n"   "N" },
        {SDLK_O,            "o"   "O" },
        {SDLK_P,            "p"   "P" },
        {SDLK_Q,            "q"   "Q" },
        {SDLK_R,            "r"   "R" },
        {SDLK_S,            "s"   "S" },
        {SDLK_T,            "t"   "T" },
        {SDLK_U,            "u"   "U" },
        {SDLK_V,            "v"   "V" },
        {SDLK_W,            "w"   "W" },
        {SDLK_X,            "x"   "X" },
        {SDLK_Y,            "y"   "Y" },
        {SDLK_Z,            "z"   "Z" },
    };

    if(const auto p = s_lookupTable.find(event.key.key); p != s_lookupTable.end()){
        return p->second[checkShiftKey && ((event.key.mod & SDL_KMOD_LSHIFT) || (event.key.mod & SDL_KMOD_RSHIFT)) ? 1 : 0];
    }
    return '\0';
}

SDLDeviceHelper::SDLEventPLoc SDLDeviceHelper::getMousePLoc()
{
    float mousePX = -1.0f;
    float mousePY = -1.0f;
    SDL_GetMouseState(&mousePX, &mousePY);

    return
    {
        to_d(mousePX),
        to_d(mousePY),
    };
}

std::tuple<int, int, Uint32> SDLDeviceHelper::getMouseState()
{
    float mousePX = -1.0f;
    float mousePY = -1.0f;
    SDL_MouseButtonFlags mouseState = SDL_GetMouseState(&mousePX, &mousePY);

    return
    {
        to_d(mousePX),
        to_d(mousePY),
        static_cast<Uint32>(mouseState),
    };
}

std::optional<SDLDeviceHelper::SDLEventPLoc> SDLDeviceHelper::getEventPLoc(const SDL_Event &event)
{
    switch(event.type){
        case SDL_EVENT_MOUSE_MOTION:
            {
                return SDLDeviceHelper::SDLEventPLoc
                {
                    to_d(event.motion.x),
                    to_d(event.motion.y),
                };
            }
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                return SDLDeviceHelper::SDLEventPLoc
                {
                    to_d(event.button.x),
                    to_d(event.button.y),
                };
            }
        case SDL_EVENT_MOUSE_WHEEL:
            {
                return SDLDeviceHelper::SDLEventPLoc
                {
                    to_d(event.wheel.mouse_x),
                    to_d(event.wheel.mouse_y),
                };
            }
        default:
            {
                return {};
            }
    }
}

std::tuple<int, int> SDLDeviceHelper::getTextureSize(const SDL_Texture *texture)
{
    fflassert(texture);
    return {texture->w, texture->h};
}

int SDLDeviceHelper::getTextureWidth(const SDL_Texture *texture, std::optional<int> optW)
{
    if(texture){
        return texture->w;
    }
    else if(optW.has_value() && optW.value() >= 0){
        return optW.value();
    }
    else{
        throw fflpanic("invalid texture pointer and optional width");
    }
}

int SDLDeviceHelper::getTextureHeight(const SDL_Texture *texture, std::optional<int> optH)
{
    if(texture){
        return texture->h;
    }
    else if(optH.has_value() && optH.value() >= 0){
        return optH.value();
    }
    else{
        throw fflpanic("invalid texture pointer and optional height");
    }
}

SDLSoundEffectChannel::SDLSoundEffectChannel(SDLDevice *sdlDevice, MIX_Track *track)
    : m_sdlDevice(sdlDevice)
    , m_track(track)
{
    fflassert(m_sdlDevice);
    fflassert(m_track);
}

SDLSoundEffectChannel::~SDLSoundEffectChannel()
{
    if(m_track){
        if(auto p = m_sdlDevice->m_trackStateList.find(m_track); p != m_sdlDevice->m_trackStateList.end()){
            if(const auto hooked = std::exchange(p->second.hooked, false); !hooked){
                g_mir2xLog->addLog(LOGTYPE_WARNING, "Sound track gets unhooked unexpectedly: %p", to_cvptr(m_track));
            }
        }
        else{
            g_mir2xLog->addLog(LOGTYPE_WARNING, "Sound track has no associated state: %p", to_cvptr(m_track));
        }
    }
}

void SDLSoundEffectChannel::halt()
{
    if(m_track){
        MIX_StopTrack(m_track, 0);
        m_sdlDevice->m_trackStateList.erase(m_track);
        m_track = nullptr;
    }
}

void SDLSoundEffectChannel::pause()
{
    fflassert(m_track);
    MIX_PauseTrack(m_track);
}

void SDLSoundEffectChannel::resume()
{
    fflassert(m_track);
    MIX_ResumeTrack(m_track);
}

void SDLSoundEffectChannel::setPosition(int distance, int angle)
{
    fflassert(m_track);
    fflassert(distance >= 0, distance);
    SDLDevice::apply3DPosition(m_track, std::min<int>(distance, 255), ((angle % 360) + 360) % 360);
}

SDLDevice::SDLDevice()
{
    if(g_sdlDevice){
        throw fflpanic("multiple initialization for SDLDevice");
    }

    SDL_SetMainReady();
    if(!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS)){
        throw fflpanic("initialization failed for SDL3: {}", SDL_GetError());
    }

    if(!TTF_Init()){
        throw fflpanic("initialization failed for SDL3_ttf: {}", SDL_GetError());
    }

    // SDL3_image dropped IMG_Init/IMG_Quit — no explicit init needed.

    if(!g_clientArgParser->disableAudio){
        if(!MIX_Init()){
            throw fflpanic("initialization failed for SDL3_mixer: {}", SDL_GetError());
        }

        m_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if(!m_mixer){
            throw fflpanic("MIX_CreateMixerDevice failed: {}", SDL_GetError());
        }

#if defined linux
        if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0")){
            throw fflpanic("SDL failed to disable compositor bypass");
        }
#endif

        m_bgmTrack = MIX_CreateTrack(m_mixer);
        if(!m_bgmTrack){
            throw fflpanic("MIX_CreateTrack(bgm) failed: {}", SDL_GetError());
        }

        m_tracks.reserve(m_trackCount);
        for(size_t i = 0; i < m_trackCount; ++i){
            auto *t = MIX_CreateTrack(m_mixer);
            if(!t){
                throw fflpanic("MIX_CreateTrack failed: {}", SDL_GetError());
            }
            m_tracks.push_back(t);
            m_freeTrackList.insert(t);
            MIX_SetTrackStoppedCallback(t, &SDLDevice::recycleSoundEffectTrack, this);
        }
    }
}

SDLDevice::~SDLDevice()
{
    if(!g_clientArgParser->disableAudio){
        for(auto *t : m_tracks){
            MIX_DestroyTrack(t);
        }
        m_tracks.clear();
        m_freeTrackList.clear();
        m_trackStateList.clear();

        if(m_bgmTrack){
            MIX_DestroyTrack(m_bgmTrack);
            m_bgmTrack = nullptr;
        }

        if(m_mixer){
            MIX_DestroyMixer(m_mixer);
            m_mixer = nullptr;
        }
        MIX_Quit();
    }

    for(auto p: m_fontList){
        TTF_CloseFont(p.second);
    }
    m_fontList.clear();

    for(auto p: m_cover){
        SDL_DestroyTexture(p.second);
    }
    m_cover.clear();

    // Reset explicitly before SDL_Quit so the SDL handles release while the
    // SDL subsystems are still alive. (Members otherwise destruct after this
    // function returns, i.e. AFTER SDL_Quit — which would UB.)
    m_renderer.reset();
    m_window.reset();

    SDL_Quit();
}

void SDLDevice::setWindowIcon()
{
    constexpr uint8_t winIconData []
    {
        #embed "winicon.png"
    };

    SDL_IOStream *ioStream = nullptr;
    SDL_Surface  *surfPtr  = nullptr;

    if((ioStream = SDL_IOFromConstMem(std::data(winIconData), std::size(winIconData)))){
        if((surfPtr = IMG_LoadPNG_IO(ioStream))){
            SDL_SetWindowIcon(m_window.get(), surfPtr);
        }
    }

    if(ioStream){
        SDL_CloseIO(ioStream);
    }

    if(surfPtr){
        SDL_DestroySurface(surfPtr);
    }
}

void SDLDevice::toggleWindowFullscreen()
{
    fflassert(m_window);
    const auto winFlag = SDL_GetWindowFlags(m_window.get());

    if(winFlag & SDL_WINDOW_FULLSCREEN){
        if(!SDL_SetWindowFullscreen(m_window.get(), false)){
            throw fflpanic("SDL_SetWindowFullscreen({:p}) failed: {}", to_cvptr(m_window.get()), SDL_GetError());
        }
    }
    else{
        if(!SDL_SetWindowFullscreen(m_window.get(), true)){
            throw fflpanic("SDL_SetWindowFullscreen({:p}) failed: {}", to_cvptr(m_window.get()), SDL_GetError());
        }
    }
}

SDL_Texture *SDLDevice::loadPNGTexture(const void *data, size_t size)
{
    fflassert(data);
    fflassert(size > 0);

    if(!m_renderer){
        return nullptr;
    }

    if(auto ioStream = SDL_IOFromConstMem(data, size)){
        return IMG_LoadTextureTyped_IO(m_renderer.get(), ioStream, true, "PNG");
    }
    return nullptr;
}

void SDLDevice::drawTexture(SDL_Texture *texPtr,
        int dstX, int dstY,
        int dstW, int dstH,
        int srcX, int srcY,
        int srcW, int srcH)
{
    if(texPtr){
        const SDL_FRect src {to_f(srcX), to_f(srcY), to_f(srcW), to_f(srcH)};
        const SDL_FRect dst {to_f(dstX), to_f(dstY), to_f(dstW), to_f(dstH)};

        SDL_RenderTexture(m_renderer.get(), texPtr, &src, &dst);
        if(g_clientArgParser->debugDrawTexture){
            drawRectangle(colorf::BLUE + colorf::A_SHF(128), dstX, dstY, dstW, dstH);
        }
    }
}

void SDLDevice::drawTexture(SDL_Texture *texPtr,
        int dstX, int dstY,
        int srcX, int srcY,
        int srcW, int srcH)
{
    drawTexture(texPtr, dstX, dstY, srcW, srcH, srcX, srcY, srcW, srcH);
}

void SDLDevice::drawTexture(SDL_Texture *texPtr, int dstX, int dstY)
{
    if(texPtr){
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        drawTexture(texPtr, dstX, dstY, 0, 0, texW, texH);
    }
}

void SDLDevice::drawTexture(SDL_Texture *texPtr, dir8_t dir, int anchorX, int anchorY)
{
    if(texPtr){
        const auto [dstX, dstY] = [texPtr, dir, anchorX, anchorY]() -> std::tuple<int, int>
        {
            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
            switch(dir){
                case DIR_UPLEFT   : return {anchorX           , anchorY           };
                case DIR_UP       : return {anchorX - texW / 2, anchorY           };
                case DIR_UPRIGHT  : return {anchorX - texW - 1, anchorY           };
                case DIR_RIGHT    : return {anchorX - texW - 1, anchorY - texH / 2};
                case DIR_DOWNRIGHT: return {anchorX - texW - 1, anchorY - texH - 1};
                case DIR_DOWN     : return {anchorX - texW / 2, anchorY - texH - 1};
                case DIR_DOWNLEFT : return {anchorX           , anchorY - texH - 1};
                case DIR_LEFT     : return {anchorX           , anchorY - texH / 2};
                default           : return {anchorX - texW / 2, anchorY - texH / 2};
            }
        }();
        drawTexture(texPtr, dstX, dstY);
    }
}

TTF_Font *SDLDevice::createTTF(const void *data, size_t size, uint8_t fontPtSize)
{
    fflassert(data);
    fflassert(size > 0);
    fflassert(fontPtSize > 0);

    if(auto ioStream = SDL_IOFromConstMem(data, size)){
        // closeio=true: TTF will close the stream when the font is closed
        return TTF_OpenFontIO(ioStream, true, to_f(fontPtSize));
    }
    return nullptr;
}

void SDLDevice::createInitViewWindow()
{
    // Reset renderer before window (renderer references the window)
    m_renderer.reset();
    m_window.reset();

    int windowW = 800;
    int windowH = 600;
    {
        if(const auto *desktop = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay())){
            windowW = std::min<int>(windowW, desktop->w);
            windowH = std::min<int>(windowH, desktop->h);
        }
    }

    m_window.reset(SDL_CreateWindow("MIR2X-V0.1-LOADING", windowW, windowH, SDL_WINDOW_BORDERLESS));
    if(!m_window){
        throw fflpanic("failed to create SDL window handler: {}", SDL_GetError());
    }

    SDL_ShowWindow(m_window.get());
    SDL_RaiseWindow(m_window.get());
    SDL_SetWindowResizable(m_window.get(), false);

    m_renderer.reset(SDL_CreateRenderer(m_window.get(), nullptr));
    if(!m_renderer){
        m_window.reset();
        throw fflpanic("failed to create SDL renderer: {}", SDL_GetError());
    }

    setWindowIcon();

    if(!SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 255)){
        throw fflpanic("set renderer draw color failed: {}", SDL_GetError());
    }

    if(!SDL_SetRenderDrawBlendMode(m_renderer.get(), SDL_BLENDMODE_BLEND)){
        throw fflpanic("set renderer blend mode failed: {}", SDL_GetError());
    }
}

void SDLDevice::createMainWindow()
{
    m_renderer.reset();
    m_window.reset();

    const auto winFlag = []() -> Uint32
    {
        // SDL3 dropped SDL_WINDOW_FULLSCREEN_DESKTOP; fullscreen mode is now
        // configured via SDL_SetWindowFullscreenMode after window creation.
        switch(g_clientArgParser->screenMode){
            case  1: return SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN;
            case  2: return SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN;
            default: return SDL_WINDOW_RESIZABLE;
        }
    }();

    m_window.reset(SDL_CreateWindow("MIR2X-V0.1", SYS_WINDOW_MIN_W, SYS_WINDOW_MIN_H, winFlag));
    fflassert(m_window);

    SDL_SetWindowMinimumSize(m_window.get(), SYS_WINDOW_MIN_W, SYS_WINDOW_MIN_H);
    m_renderer.reset(SDL_CreateRenderer(m_window.get(), nullptr));

    if(!m_renderer){
        m_window.reset();
        throw fflpanic("failed to create SDL renderer: {}", SDL_GetError());
    }

    setWindowIcon();

    if(!SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 0)){
        throw fflpanic("set renderer draw color failed: {}", SDL_GetError());
    }

    if(!SDL_SetRenderDrawBlendMode(m_renderer.get(), SDL_BLENDMODE_BLEND)){
        throw fflpanic("set renderer blend mode failed: {}", SDL_GetError());
    }
}

void SDLDevice::drawTextureEx(
        SDL_Texture *texPtr,

        int srcX, int srcY,
        int srcW, int srcH,

        int dstX, int dstY,
        int dstW, int dstH,

        int centerDstOffX,
        int centerDstOffY,

        int rotateDegree,
        SDL_FlipMode flip)
{
    if(texPtr){
        const SDL_FRect src {to_f(srcX), to_f(srcY), to_f(srcW), to_f(srcH)};
        const SDL_FRect dst {to_f(dstX), to_f(dstY), to_f(dstW), to_f(dstH)};
        const SDL_FPoint center {to_f(centerDstOffX), to_f(centerDstOffY)};
        const double angle = 1.00 * (rotateDegree % 360);
        if(!SDL_RenderTextureRotated(m_renderer.get(), texPtr, &src, &dst, angle, &center, flip)){
            throw fflpanic("SDL_RenderTextureRotated({:p}) failed: {}", to_cvptr(m_renderer.get()), SDL_GetError());
        }

        if(g_clientArgParser->debugDrawTexture){
            const double radian = -1.0 * angle / 180.0 * mathf::pi;

            const auto fnRotatePoint = [x0 = dstX + centerDstOffX, y0 = dstY + centerDstOffY, radian](double x, double y) -> std::pair<double, double>
            {
                const double dx =  x - x0;
                const double dy = -y + y0;

                return
                {
                    x0 + (std::cos(radian) * dx - std::sin(radian) * dy),
                    y0 - (std::sin(radian) * dx + std::cos(radian) * dy),
                };
            };

            const auto rp0 = fnRotatePoint(dstX            + 0.5, dstY            + 0.5);
            const auto rp1 = fnRotatePoint(dstX + dstW - 1 + 0.5, dstY            + 0.5);
            const auto rp2 = fnRotatePoint(dstX + dstW - 1 + 0.5, dstY + dstH - 1 + 0.5);
            const auto rp3 = fnRotatePoint(dstX            + 0.5, dstY + dstH - 1 + 0.5);

            drawLinef(colorf::BLUE + colorf::A_SHF(128), rp0.first, rp0.second, rp1.first, rp1.second);
            drawLinef(colorf::BLUE + colorf::A_SHF(128), rp1.first, rp1.second, rp2.first, rp2.second);
            drawLinef(colorf::BLUE + colorf::A_SHF(128), rp2.first, rp2.second, rp3.first, rp3.second);
            drawLinef(colorf::BLUE + colorf::A_SHF(128), rp3.first, rp3.second, rp0.first, rp0.second);
        }
    }
}

SDL_Texture *SDLDevice::createRGBATexture(const uint32_t *data, size_t w, size_t h)
{
    fflassert(data);
    fflassert(w > 0);
    fflassert(h > 0);

    if(auto texPtr = SDL_CreateTexture(m_renderer.get(), SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, w, h)){
        if(SDL_UpdateTexture(texPtr, nullptr, data, w * 4) && SDL_SetTextureBlendMode(texPtr, SDL_BLENDMODE_BLEND)){
            return texPtr;
        }
        SDL_DestroyTexture(texPtr);
    }
    return nullptr;
}

TTF_Font *SDLDevice::defaultTTF(uint8_t fontSize)
{
    if(auto p = m_fontList.find(fontSize); p != m_fontList.end()){
        return p->second;
    }

    // TTF_OpenFontIO with closeio=true holds onto the IO stream and re-reads font data on demand (glyph cache misses, etc.)
    // so the buffer must outlive the TTF_Font

    // A plain `constexpr` local has automatic storage duration despite the name
    // The font would point into reused stack memory and rendering would intermittently fail

    constexpr static uint8_t ttfData []
    {
        #embed "monaco.ttf"
    };

    if(auto ttfPtr = createTTF(std::data(ttfData), std::size(ttfData), fontSize); ttfPtr){
        return m_fontList[fontSize] = ttfPtr;
    }
    throw fflpanic("can't build default ttf with point: {}", fontSize);
}

SDL_Texture *SDLDevice::getCover(int r, int angle)
{
    fflassert(r > 0);
    fflassert(angle >= 0 && angle <= 360);

    const int key = r * 360 + angle;
    if(auto p = m_cover.find(key); p != m_cover.end()){
        if(p->second){
            return p->second;
        }
        throw fflpanic("invalid registered cover: r = {}, angle = {}", r, angle);
    }

    const int w = r * 2 - 1;
    const int h = r * 2 - 1;

    std::vector<uint32_t> buf(w * h);
    for(int y = 0; y < h; ++y){
        for(int x = 0; x < w; ++x){
            const int dx =  1 * (x - r + 1);
            const int dy = -1 * (y - r + 1);
            const int curr_r2 = dx * dx + dy * dy;
            const uint8_t alpha = [curr_r2, r]() -> uint8_t
            {
                if(g_clientArgParser->debugAlphaCover){
                    return 255;
                }
                return 255 - std::min<uint8_t>(255, to_dround(255.0 * curr_r2 / (r * r)));
            }();

            const auto curr_angle = [dx, dy]() -> int
            {
                if(dx == 0){
                    return (dy >= 0) ? 0 : 180;
                }
                return ((dx > 0) ? 0 : 180) + to_d(std::lround((1.0 - 2.0 * std::atan(to_df(dy) / dx) / 3.14159265358979323846) * 90.0));
            }();

            if(curr_r2 < r * r && std::clamp<int>(curr_angle, 0, 360) <= angle){
                buf[x + y * w] = colorf::RGBA(0XFF, 0XFF, 0XFF, alpha);
            }
            else{
                buf[x + y * w] = colorf::RGBA(0, 0, 0, 0);
            }
        }
    }

    if(auto texPtr = createRGBATexture(buf.data(), w, h)){
        return m_cover[key] = texPtr;
    }
    throw fflpanic("failed to create texture: r = {}, angle = {}", r, angle);
}

void SDLDevice::fillRectangle(int argX, int argY, int argW, int argH, int argRad)
{
    if(!(argW > 0 && argH > 0)){
        return;
    }

    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
    Uint8 a = 0;

    if(!SDL_GetRenderDrawColor(getRenderer(), &r, &g, &b, &a)){
        throw fflpanic("get renderer draw color failed: {}", SDL_GetError());
    }

    if(a == 0){
        return;
    }

    if(!roundedBoxRGBA(getRenderer(), argX, argY, argX + argW - 1, argY + argH - 1, argRad, r, g, b, a)){
        throw fflpanic("roundedRectangleRGBA() failed");
    }
}

void SDLDevice::fillRectangle(uint32_t nRGBA, int argX, int argY, int argW, int argH, int argRad)
{
    if(!(argW > 0 && argH > 0)){
        return;
    }

    if(colorf::A(nRGBA)){
        SDLDeviceHelper::EnableRenderColor enableColor(nRGBA, this);
        SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
        fillRectangle(argX, argY, argW, argH, argRad);
    }
}

void SDLDevice::drawCircle(uint32_t color, int argX, int argY, int argRad)
{
    fflassert(argRad >= 0, argRad);
    if(colorf::A(color)){
        aacircleRGBA(m_renderer.get(), argX, argY, argRad, colorf::R(color), colorf::G(color), colorf::B(color), colorf::A(color));
    }
}

void SDLDevice::fillCircle(uint32_t color, int argX, int argY, int argRad)
{
    fflassert(argRad >= 0, argRad);
    if(colorf::A(color)){
        filledCircleRGBA(m_renderer.get(), argX, argY, argRad, colorf::R(color), colorf::G(color), colorf::B(color), colorf::A(color));
    }
}

void SDLDevice::drawTriangle(int argX1, int argY1, int argX2, int argY2, int argX3, int argY3)
{
    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
    Uint8 a = 0;

    if(!SDL_GetRenderDrawColor(getRenderer(), &r, &g, &b, &a)){
        throw fflpanic("get renderer draw color failed: {}", SDL_GetError());
    }

    if(a == 0){
        return;
    }

    if(!aatrigonRGBA(getRenderer(), argX1, argY1, argX2, argY2, argX3, argY3, r, g, b, a)){
        throw fflpanic("aatrigonRGBA() failed");
    }
}

void SDLDevice::drawTriangle(uint32_t argColor, int argX1, int argY1, int argX2, int argY2, int argX3, int argY3)
{
    if(colorf::A(argColor)){
        SDLDeviceHelper::EnableRenderColor enableColor(argColor, this);
        SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
        drawTriangle(argX1, argY1, argX2, argY2, argX3, argY3);
    }
}

void SDLDevice::fillTriangle(int argX1, int argY1, int argX2, int argY2, int argX3, int argY3)
{
    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
    Uint8 a = 0;

    if(!SDL_GetRenderDrawColor(getRenderer(), &r, &g, &b, &a)){
        throw fflpanic("get renderer draw color failed: {}", SDL_GetError());
    }

    if(a == 0){
        return;
    }

    if(!filledTrigonRGBA(getRenderer(), argX1, argY1, argX2, argY2, argX3, argY3, r, g, b, a)){
        throw fflpanic("filledTrigonRGBA() failed");
    }
}

void SDLDevice::fillTriangle(uint32_t argColor, int argX1, int argY1, int argX2, int argY2, int argX3, int argY3)
{
    if(colorf::A(argColor)){
        SDLDeviceHelper::EnableRenderColor enableColor(argColor, this);
        SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
        fillTriangle(argX1, argY1, argX2, argY2, argX3, argY3);
    }
}

void SDLDevice::drawRectangle(int argX, int argY, int argW, int argH, int argRad)
{
    if(argRad <= 1){
        argW++;
        argH++;
    }

    if(!(argW > 0 && argH > 0)){
        return;
    }

    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
    Uint8 a = 0;

    if(!SDL_GetRenderDrawColor(getRenderer(), &r, &g, &b, &a)){
        throw fflpanic("get renderer draw color failed: {}", SDL_GetError());
    }

    if(a == 0){
        return;
    }

    if(!roundedRectangleRGBA(getRenderer(), argX, argY, argX + argW - 1, argY + argH - 1, argRad, r, g, b, a)){
        throw fflpanic("roundedRectangleRGBA() failed");
    }
}

void SDLDevice::drawRectangle(uint32_t color, int argX, int argY, int argW, int argH, int argRad)
{
    if(!(argW > 0 && argH > 0)){
        return;
    }

    if(colorf::A(color)){
        SDLDeviceHelper::EnableRenderColor enableColor(color, this);
        SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
        drawRectangle(argX, argY, argW, argH, argRad);
    }
}

void SDLDevice::drawWidthRectangle(size_t frameLineWidth, int argX, int argY, int argW, int argH)
{
    if(!frameLineWidth){
        return;
    }

    if(frameLineWidth == 1){
        drawRectangle(argX, argY, argW, argH);
        return;
    }

    fillRectangle(argX, argY,                             argW, frameLineWidth);
    fillRectangle(argX, argY + argW - 2 * frameLineWidth, argW, frameLineWidth);

    fillRectangle(argX,                             argY + frameLineWidth, frameLineWidth, argH - 2 * frameLineWidth);
    fillRectangle(argX + argW - 2 * frameLineWidth, argY + frameLineWidth, frameLineWidth, argH - 2 * frameLineWidth);
}

void SDLDevice::drawWidthRectangle(uint32_t color, size_t frameLineWidth, int argX, int argY, int argW, int argH)
{
    SDLDeviceHelper::EnableRenderColor enableColor(color, this);
    SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
    drawWidthRectangle(frameLineWidth, argX, argY, argW, argH);
}

void SDLDevice::drawHLineFading(uint32_t startColor, uint32_t endColor, int x, int y, int length)
{
    if(length){
        SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
        for(int i = 0; i < std::abs(length); ++i){
            SDLDeviceHelper::EnableRenderColor enableColor(colorf::fadeRGBA(startColor, endColor, i * 1.0f / std::abs(length)), this);
            SDL_RenderPoint(getRenderer(), to_f(x + i * (length > 0 ? 1 : -1)), to_f(y));
        }
    }
}

void SDLDevice::drawVLineFading(uint32_t startColor, uint32_t endColor, int x, int y, int length)
{
    if(length){
        SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
        for(int i = 0; i < std::abs(length); ++i){
            SDLDeviceHelper::EnableRenderColor enableColor(colorf::fadeRGBA(startColor, endColor, i * 1.0f / std::abs(length)), this);
            SDL_RenderPoint(getRenderer(), to_f(x), to_f(y + i * (length > 0 ? 1 : -1)));
        }
    }
}

void SDLDevice::drawBoxFading(uint32_t startColor, uint32_t endColor, int x, int y, int w, int h, int start, int length)
{
    if((w == 0) || (h == 0) || (length == 0)){
        return;
    }

    SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
    if(w < 0){
        w  = -w;
        x -=  w;
    }

    if(h < 0){
        h  = -h;
        y -=  h;
    }

    const auto edgeGridCount = [w, h]() -> int
    {
        if(w == 1 && h == 1){
            return 1;
        }

        if(w == 1 || h == 1){
            return std::max<int>(w, h);
        }

        return (w + h) * 2 - 4;
    }();

    const auto fnMod = [edgeGridCount](int offset)
    {
        return ((offset % edgeGridCount) + edgeGridCount) % edgeGridCount;
    };

    if(length < 0){
        start = fnMod(start + length);
    }
    else{
        start = fnMod(start);
    }

    start = fnMod(start - length + 1);
    length = std::min<int>(std::abs(length), edgeGridCount);

    const auto endX = x + w - 1;
    const auto endY = y + h - 1;

    auto [currX, currY, currDir] = [x, y, w, h, start, endX, endY]() -> std::tuple<int, int, int>
    {
        if((w == 1) && (h == 1)){
            return {x, y, 3};
        }
        else if(w == 1){
            if(start < h){
                return {x, y + start, 3};
            }
            else{
                return {x, endY - (start - h), 1};
            }
        }
        else if(h == 1){
            if(start < w){
                return {x + start, y, 0};
            }
            else{
                return {endX - (start - w), y, 2};
            }
        }
        else if(start < h){
            return {x, y + start, 3};
        }
        else if(start < w + h - 1){
            return {x + (start - (h - 1)), endY, 0};
        }
        else if(start < w + h * 2 - 2){
            return {endX, endY - (start - (w - 1) - (h - 1)), 1};
        }
        else if(start < w * 2 + h * 2 - 3){
            return {endX - (start - (h - 1) - (w - 1) - (h - 1)), y, 2};
        }
        else{
            throw fflvalue(x, y, w, h, start);
        }
    }();

    const auto fnNextPoint = [x, y, w, h, endX, endY](int currX, int currY, int dir) -> std::tuple<int, int, int>
    {
        switch(dir){
            case 0:
                {
                    if(currX == endX){
                        if(h == 1){
                            return {currX - 1, currY, 2};
                        }
                        else{
                            return {currX, currY - 1, 1};
                        }
                    }
                    else{
                        return {currX + 1, currY, 0};
                    }
                }
            case 1:
                {
                    if(currY == y){
                        if(w == 1){
                            return {currX, currY + 1, 3};
                        }
                        else{
                            return {currX - 1, currY, 2};
                        }
                    }
                    else{
                        return {currX, currY - 1, 1};
                    }
                }
            case 2:
                {
                    if(currX == x){
                        if(h == 1){
                            return {currX + 1, currY, 0};
                        }
                        else{
                            return {currX, currY + 1, 3};
                        }
                    }
                    else{
                        return {currX - 1, currY, 2};
                    }
                }
            case 3:
                {
                    if(currY == endY){
                        if(w == 1){
                            return {currX, currY - 1, 1};
                        }
                        else{
                            return {currX + 1, currY, 0};
                        }
                    }
                    else{
                        return {currX, currY + 1, 3};
                    }
                }
            default:
                {
                    throw fflvalue(dir, currX, currY);
                }
        }
    };

    for(int i = 0; i < length; ++i){
        SDLDeviceHelper::EnableRenderColor enableColor(colorf::fadeRGBA(startColor, endColor,  1.0 - (1.0 * i / length)), this);
        SDL_RenderPoint(getRenderer(), to_f(currX), to_f(currY));
        std::tie(currX, currY, currDir) = fnNextPoint(currX, currY, currDir);
    }
}

void SDLDevice::drawString(uint32_t color, int x, int y, const char *s)
{
    if(!stringRGBA(m_renderer.get(), x, y, s, colorf::R(color), colorf::G(color), colorf::B(color), colorf::A(color))){
        throw fflpanic("failed to draw 8x8 string: {}", s);
    }
}

void SDLDevice::setWindowSize(int w, int h)
{
    fflassert(w >= 400, w, h);
    fflassert(h >= 400, w, h);

    SDL_SetWindowSize(m_window.get(), w, h);
}

void SDLDevice::stopBGM()
{
    if(g_clientArgParser->disableAudio){
        return;
    }
    if(m_bgmTrack){
        MIX_StopTrack(m_bgmTrack, 0);
    }
}

void SDLDevice::setBGMVolume(float volume)
{
    if(g_clientArgParser->disableAudio){
        return;
    }
    if(m_bgmTrack){
        MIX_SetTrackGain(m_bgmTrack, mathf::bound<float>(volume, 0.0f, 1.0f));
    }
}

void SDLDevice::playBGM(MIX_Audio *music, size_t repeats)
{
    if(g_clientArgParser->disableAudio){
        return;
    }

    if(!music || !m_bgmTrack){
        return;
    }

    if(!MIX_SetTrackAudio(m_bgmTrack, music)){
        throw fflpanic("MIX_SetTrackAudio failed: {}", SDL_GetError());
    }

    // repeats == 0 in old API meant "loop forever" → -1
    // repeats == N meant "play N times" → loops = N - 1
    const int loops = (repeats == 0) ? -1 : (to_d(repeats) - 1);

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
    const bool ok = MIX_PlayTrack(m_bgmTrack, props);
    SDL_DestroyProperties(props);

    if(!ok){
        throw fflpanic("failed to play music: {}", SDL_GetError());
    }
}

std::shared_ptr<SDLSoundEffectChannel> SDLDevice::playSoundEffect(std::shared_ptr<SoundEffectHandle> handle, int distance, int angle, size_t repeats)
{
    if(g_clientArgParser->disableAudio){
        return {};
    }

    if(!handle || !handle->audio){
        return {};
    }

    if(distance < 0){
        return {};
    }

    MIX_Track *pickedTrack = nullptr;
    std::inplace_vector<MIX_Track *, 128> idleTrackList;
    {
        // clean pending track entries
        //
        // a free track that still has a hooked SoundChannelHookState is one whose
        // SDLSoundEffectChannel shared_ptr hasn't been destroyed yet — it must not be
        // reused before that owner releases.

        const std::lock_guard<std::mutex> lockGuard(m_freeTrackLock);
        for(auto *t : m_freeTrackList){
            if(auto p = m_trackStateList.find(t); (p != m_trackStateList.end()) && p->second.hooked){
                continue;
            }
            else if(idleTrackList.size() < idleTrackList.capacity()){
                idleTrackList.push_back(t);
            }
        }

        if(idleTrackList.empty()){
            return {};
        }

        pickedTrack = idleTrackList.back();
        m_freeTrackList.erase(pickedTrack);
    }

    for(auto *idleTrack : idleTrackList){
        m_trackStateList.erase(idleTrack);
    }

    fflassert(pickedTrack);
    m_trackStateList.try_emplace(pickedTrack, SoundChannelHookState{.hooked = true, .handle = handle});

    MIX_StopTrack(pickedTrack, 0);
    apply3DPosition(pickedTrack, std::min<int>(distance, 255), ((angle % 360) + 360) % 360);

    if(!MIX_SetTrackAudio(pickedTrack, handle->audio)){
        throw fflpanic("MIX_SetTrackAudio failed: {}", SDL_GetError());
    }

    const int loops = (repeats == 0) ? -1 : (to_d(repeats) - 1);

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
    const bool ok = MIX_PlayTrack(pickedTrack, props);
    SDL_DestroyProperties(props);

    if(!ok){
        throw fflpanic("MIX_PlayTrack failed: {}", SDL_GetError());
    }

    return std::shared_ptr<SDLSoundEffectChannel>(new SDLSoundEffectChannel
    {
        this,
        pickedTrack,
    });
}

void SDLDevice::stopSoundEffect()
{
    if(g_clientArgParser->disableAudio){
        return;
    }
    // SDL2 Mix_HaltChannel(-1) only halted sound-effect channels (music was separate).
    // MIX_StopAllTracks would also halt m_bgmTrack — iterate the effect tracks instead.
    for(auto *t : m_tracks){
        MIX_StopTrack(t, 0);
    }
}

void SDLDevice::setSoundEffectVolume(float volume)
{
    if(g_clientArgParser->disableAudio){
        return;
    }

    // SDL3_mixer has no per-bus gain control (MIX_Group exists but only for
    // routing/post-mix callbacks, not volume). Per-track gain compounds with
    // master gain and is the proper way to scale a subset of tracks: applying
    // it to every SFX track in the pool leaves m_bgmTrack untouched, so BGM
    // and SFX volumes are independent.
    const float gain = mathf::bound<float>(volume, 0.0f, 1.0f);
    for(auto *t : m_tracks){
        MIX_SetTrackGain(t, gain);
    }
}

void SDLDevice::recycleSoundEffectTrack(void *userdata, MIX_Track *track)
{
    auto *self = static_cast<SDLDevice *>(userdata);
    if(g_clientArgParser->disableAudio){
        return;
    }

    // only put the track back to the free list
    // don't reset the handle here — release happens when track is next allocated

    const std::lock_guard<std::mutex> lockGuard(self->m_freeTrackLock);
    self->m_freeTrackList.insert(track);
}

void SDLDevice::apply3DPosition(MIX_Track *track, int distance, int angle)
{
    // angle in [0, 360), 0=north (-z), 90=east (+x), 180=south (+z), 270=west (-x)
    // distance in [0, 255], 0 = at listener
    const double rad = angle * mathf::pi / 180.0;
    const float scaled = to_f(distance) / 255.0f;
    MIX_Point3D pos {
        to_f(std::sin(rad) * scaled),
        0.0f,
        to_f(-std::cos(rad) * scaled),
    };
    MIX_SetTrack3DPosition(track, &pos);
}
