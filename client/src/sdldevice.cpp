#include <ranges>
#include <numeric>
#include <cinttypes>
#include <system_error>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

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
#include "scopedalloc.hpp"

extern Log *g_log;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

SDLDeviceHelper::EnableRenderColor::EnableRenderColor(uint32_t color, SDLDevice *devPtr)
    : m_device(devPtr ? devPtr : g_sdlDevice)
{
    if(SDL_GetRenderDrawColor(m_device->getRenderer(), &m_r, &m_g, &m_b, &m_a)){
        throw fflerror("get renderer draw color failed: %s", SDL_GetError());
    }

    if(SDL_SetRenderDrawColor(m_device->getRenderer(), colorf::R(color), colorf::G(color), colorf::B(color), colorf::A(color))){
        throw fflerror("set renderer draw color failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderColor::~EnableRenderColor()
{
    if(SDL_SetRenderDrawColor(m_device->getRenderer(), m_r, m_g, m_b, m_a)){
        g_log->addLog(LOGTYPE_WARNING, "Set renderer draw color failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderBlendMode::EnableRenderBlendMode(SDL_BlendMode blendMode, SDLDevice *devPtr)
    : m_device(devPtr ? devPtr : g_sdlDevice)
{
    if(SDL_GetRenderDrawBlendMode(m_device->getRenderer(), &m_blendMode)){
        throw fflerror("get renderer blend mode failed: %s", SDL_GetError());
    }

    if(SDL_SetRenderDrawBlendMode(m_device->getRenderer(), blendMode)){
        throw fflerror("set renderer blend mode failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderBlendMode::~EnableRenderBlendMode()
{
    if(SDL_SetRenderDrawBlendMode(m_device->getRenderer(), m_blendMode)){
        g_log->addLog(LOGTYPE_WARNING, "set renderer blend mode failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderCropRectangle::EnableRenderCropRectangle(int x, int y, int w, int h, SDLDevice *devPtr)
    : m_device(devPtr ? devPtr : g_sdlDevice)
    , m_clipped(SDL_RenderIsClipEnabled(m_device->getRenderer()) == SDL_TRUE)
{
    if(m_clipped){
        SDL_RenderGetClipRect(m_device->getRenderer(), &m_rect);
    }

    SDL_Rect newRect;
    newRect.x = x;
    newRect.y = y;
    newRect.w = w;
    newRect.h = h;

    if(SDL_RenderSetClipRect(m_device->getRenderer(), &newRect)){
        throw fflerror("set renderer clip rectangle failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderCropRectangle::~EnableRenderCropRectangle()
{
    if(SDL_RenderSetClipRect(m_device->getRenderer(), m_clipped ? &m_rect : nullptr)){
        g_log->addLog(LOGTYPE_WARNING, "set renderer clip rectangle failed: %s", SDL_GetError());
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

    if(SDL_GetTextureBlendMode(m_texPtr, &m_blendMode)){
        throw fflerror("get texture blend mode failed: %s", SDL_GetError());
    }

    if(SDL_SetTextureBlendMode(m_texPtr, mode)){
        throw fflerror("set texture blend mode failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableTextureBlendMode::~EnableTextureBlendMode()
{
    if(!m_texPtr){
        return;
    }

    if(SDL_SetTextureBlendMode(m_texPtr, m_blendMode)){
        g_log->addLog(LOGTYPE_WARNING, "Setup texture blend mode failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableTextureModColor::EnableTextureModColor(SDL_Texture *texPtr, uint32_t color)
    : m_texPtr(texPtr)
{
    if(!m_texPtr){
        return;
    }

    if(SDL_GetTextureColorMod(m_texPtr, &m_r, &m_g, &m_b)){
        throw fflerror("SDL_GetTextureColorMod(%p) failed: %s", to_cvptr(m_texPtr), SDL_GetError());
    }

    if(SDL_GetTextureAlphaMod(m_texPtr, &m_a)){
        throw fflerror("SDL_GetTextureAlphaMod(%p) failed: %s", to_cvptr(m_texPtr), SDL_GetError());
    }

    if(SDL_SetTextureColorMod(m_texPtr, colorf::R(color), colorf::G(color), colorf::B(color))){
        throw fflerror("SDL_SetTextureColorMod(%p) failed: %s", to_cvptr(m_texPtr), SDL_GetError());
    }

    if(SDL_SetTextureAlphaMod(m_texPtr, colorf::A(color))){
        throw fflerror("SDL_SetTextureAlphaMod(%p) failed: %s", to_cvptr(m_texPtr), SDL_GetError());
    }
}

SDLDeviceHelper::EnableTextureModColor::~EnableTextureModColor()
{
    if(!m_texPtr){
        return;
    }

    if(SDL_SetTextureColorMod(m_texPtr, m_r, m_g, m_b)){
        g_log->addLog(LOGTYPE_WARNING, "set texture mod color failed: %s", SDL_GetError());
    }

    if(SDL_SetTextureAlphaMod(m_texPtr, m_a)){
        g_log->addLog(LOGTYPE_WARNING, "set texture mod alpha failed: %s", SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderTarget::EnableRenderTarget(SDL_Texture *argTarget, SDLDevice * argDevice)
    : m_device(argDevice ? argDevice : g_sdlDevice)
    , m_target(SDL_GetRenderTarget(m_device->getRenderer()))
{
    if(argTarget){
        if(int access = -1; SDL_QueryTexture(argTarget, nullptr, &access, nullptr, nullptr)){
            throw fflerror("SDL_QueryTexture(%p) failed: %s", to_cvptr(argTarget), SDL_GetError());
        }
        else if(access != SDL_TEXTUREACCESS_TARGET){
            throw fflerror("Texture can not be used as renderer target: %p", to_cvptr(argTarget));
        }
    }

    if(SDL_SetRenderTarget(m_device->getRenderer(), argTarget)){
        throw fflerror("SDL_SetRenderTarget(%p) failed: %s", to_cvptr(argTarget), SDL_GetError());
    }
}

SDLDeviceHelper::EnableRenderTarget::~EnableRenderTarget()
{
    if(SDL_SetRenderTarget(m_device->getRenderer(), m_target)){
        g_log->addLog(LOGTYPE_WARNING, "SDL_SetRenderTarget(%p) failed: %s", to_cvptr(m_target), SDL_GetError());
    }
}

char SDLDeviceHelper::getKeyChar(const SDL_Event &event, bool checkShiftKey)
{
    const static std::unordered_map<SDL_Keycode, const char *> s_lookupTable
    {
        {SDLK_SPACE,        " "   " " },
        {SDLK_QUOTE,        "'"   "\""},
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
        {SDLK_BACKQUOTE,    "`"   "~" },
        {SDLK_a,            "a"   "A" },
        {SDLK_b,            "b"   "B" },
        {SDLK_c,            "c"   "C" },
        {SDLK_d,            "d"   "D" },
        {SDLK_e,            "e"   "E" },
        {SDLK_f,            "f"   "F" },
        {SDLK_g,            "g"   "G" },
        {SDLK_h,            "h"   "H" },
        {SDLK_i,            "i"   "I" },
        {SDLK_j,            "j"   "J" },
        {SDLK_k,            "k"   "K" },
        {SDLK_l,            "l"   "L" },
        {SDLK_m,            "m"   "M" },
        {SDLK_n,            "n"   "N" },
        {SDLK_o,            "o"   "O" },
        {SDLK_p,            "p"   "P" },
        {SDLK_q,            "q"   "Q" },
        {SDLK_r,            "r"   "R" },
        {SDLK_s,            "s"   "S" },
        {SDLK_t,            "t"   "T" },
        {SDLK_u,            "u"   "U" },
        {SDLK_v,            "v"   "V" },
        {SDLK_w,            "w"   "W" },
        {SDLK_x,            "x"   "X" },
        {SDLK_y,            "y"   "Y" },
        {SDLK_z,            "z"   "Z" },
    };

    if(const auto p = s_lookupTable.find(event.key.keysym.sym); p != s_lookupTable.end()){
        return p->second[checkShiftKey && ((event.key.keysym.mod & KMOD_LSHIFT) || (event.key.keysym.mod & KMOD_RSHIFT)) ? 1 : 0];
    }
    return '\0';
}

SDLDeviceHelper::SDLEventPLoc SDLDeviceHelper::getMousePLoc()
{
    int mousePX = -1;
    int mousePY = -1;
    SDL_GetMouseState(&mousePX, &mousePY);

    return
    {
        mousePX,
        mousePY,
    };
}

std::tuple<int, int, Uint32> SDLDeviceHelper::getMouseState()
{
    int mousePX = -1;
    int mousePY = -1;
    Uint32 mouseState = SDL_GetMouseState(&mousePX, &mousePY);

    return
    {
        mousePX,
        mousePY,
        mouseState,
    };
}

std::optional<SDLDeviceHelper::SDLEventPLoc> SDLDeviceHelper::getEventPLoc(const SDL_Event &event)
{
    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                return SDLDeviceHelper::SDLEventPLoc
                {
                    event.motion.x,
                    event.motion.y,
                };
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                return SDLDeviceHelper::SDLEventPLoc
                {
                    event.button.x,
                    event.button.y,
                };
            }
        default:
            {
                return {};
            }
    }
}

std::tuple<int, int> SDLDeviceHelper::getTextureSize(SDL_Texture *texture)
{
    fflassert(texture);

    int width  = 0;
    int height = 0;

    if(!SDL_QueryTexture(const_cast<SDL_Texture *>(texture), 0, 0, &width, &height)){
        return {width, height};
    }
    throw fflerror("query texture failed: %p", to_cvptr(texture));
}

int SDLDeviceHelper::getTextureWidth(SDL_Texture *texture)
{
    return std::get<0>(getTextureSize(texture));
}

int SDLDeviceHelper::getTextureHeight(SDL_Texture *texture)
{
    return std::get<1>(getTextureSize(texture));
}

SDLSoundEffectChannel::SDLSoundEffectChannel(SDLDevice *sdlDevice, int channel)
    : m_sdlDevice(sdlDevice)
    , m_channel(channel)
{
    fflassert(m_sdlDevice);
    fflassert(m_channel >= 0, m_channel);
    fflassert(m_channel < to_d(m_sdlDevice->channelCount()), m_channel, m_sdlDevice->channelCount());
}

SDLSoundEffectChannel::~SDLSoundEffectChannel()
{
    if(m_channel >= 0){
        if(auto p = m_sdlDevice->m_channelStateList.find(m_channel); p != m_sdlDevice->m_channelStateList.end()){
            if(const auto hooked = std::exchange(p->second.hooked, false); !hooked){
                g_log->addLog(LOGTYPE_WARNING, "Sound channel gets unhooked unexpectedly: %d", m_channel);
            }
        }
        else{
            g_log->addLog(LOGTYPE_WARNING, "Sound channel has no associated state: %d", m_channel);
        }
    }
}

void SDLSoundEffectChannel::halt()
{
    if(m_channel >= 0){
        // SDL_mixer can halt a channel several times, it ignores later halt requests
        // call Mix_HaltChannel() can trigger SDLDevice::recycleSoundEffectChannel() if channel is not done yet
        // if channel has reaches its end this function is a nop
        //
        // TODO dangerous here
        //      SDLDevice should make sure a channel shall not get allocated to other sound effect before this->halt() called
        //      otherwise this function halt same channel playing other sound
        //
        // a channel has not called this->halt() must still has a slot in m_channelStateList
        // because only an explicit this->halt() call unhooks the channel, and makes it avaiable to next play request

        Mix_HaltChannel(m_channel);
        m_sdlDevice->m_channelStateList.erase(m_channel);

        m_channel = -1;
    }
}

void SDLSoundEffectChannel::pause()
{
    fflassert(m_channel >= 0);
    Mix_Pause(m_channel);
}

void SDLSoundEffectChannel::resume()
{
    fflassert(m_channel >= 0);
    Mix_Resume(m_channel);
}

void SDLSoundEffectChannel::setPosition(int distance, int angle)
{
    fflassert(m_channel >= 0);
    fflassert(distance >= 0, distance);
    Mix_SetPosition(m_channel, ((angle % 360) + 360) % 360, std::min<int>(distance, 255));
}

SDLDevice::SDLDevice()
{
    if(g_sdlDevice){
        throw fflerror("multiple initialization for SDLDevice");
    }

    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS)){
        throw fflerror("initialization failed for SDL2: %s", SDL_GetError());
    }

    if(TTF_Init()){
        throw fflerror("initialization failed for SDL2 TTF: %s", TTF_GetError());
    }

    if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG){
        throw fflerror("initialization failed for SDL2 IMG: %s", IMG_GetError());
    }

    if(!g_clientArgParser->disableAudio){
        if((Mix_Init(MIX_INIT_MP3) & MIX_INIT_MP3) != MIX_INIT_MP3){
            throw fflerror("initialization failed for SDL2 MIX: %s", Mix_GetError());
        }

        if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 512)){
            throw fflerror("initialization failed for SDL2 MIX OpenAudio: %s", Mix_GetError());
        }

#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
        if(SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0") == SDL_FALSE){
            throw fflerror("SDL failed to disable compositor bypass");
        }
#endif

        if(Mix_AllocateChannels(m_channelCount) != to_d(m_channelCount)){
            throw fflerror("failed to allocate %zu channels: %s", m_channelCount, Mix_GetError());
        }

        for(int channel = 0; channel < to_d(m_channelCount); ++channel){
            m_freeChannelList.insert(channel);
        }
        Mix_ChannelFinished(recycleSoundEffectChannel);
    }
}

SDLDevice::~SDLDevice()
{
    if(!g_clientArgParser->disableAudio){
        Mix_CloseAudio();
        Mix_Quit();
    }

    for(auto p: m_fontList){
        TTF_CloseFont(p.second);
    }
    m_fontList.clear();

    for(auto p: m_cover){
        SDL_DestroyTexture(p.second);
    }
    m_cover.clear();

    if(m_window){
        SDL_DestroyWindow(m_window);
    }

    if(m_renderer){
        SDL_DestroyRenderer(m_renderer);
    }

    SDL_Quit();
}

void SDLDevice::setWindowIcon()
{
    const static Rawbuf s_winIconBuf
    {
        #include "winicon.inc"
    };

    SDL_RWops   * rwOpsPtr = nullptr;
    SDL_Surface *  surfPtr = nullptr;

    if((rwOpsPtr = SDL_RWFromConstMem(s_winIconBuf.data(), s_winIconBuf.size()))){
        if((surfPtr = IMG_LoadPNG_RW(rwOpsPtr))){
            SDL_SetWindowIcon(m_window, surfPtr);
        }
    }

    if(rwOpsPtr){
        SDL_FreeRW(rwOpsPtr);
    }

    if(surfPtr){
        SDL_FreeSurface(surfPtr);
    }
}

void SDLDevice::toggleWindowFullscreen()
{
    fflassert(m_window);
    const auto winFlag = SDL_GetWindowFlags(m_window);

    if(winFlag & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)){
        if(SDL_SetWindowFullscreen(m_window, 0)){
            throw fflerror("SDL_SetWindowFullscreen(%p) failed: %s", to_cvptr(m_window), SDL_GetError());
        }
    }
    else{
        if(SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN)){
            throw fflerror("SDL_SetWindowFullscreen(%p) failed: %s", to_cvptr(m_window), SDL_GetError());
        }
    }
}

SDL_Texture *SDLDevice::loadPNGTexture(const void *data, size_t size)
{
    // if it's changed
    // all the texture need to be re-load

    // currently it doesn't support dynamic set of context
    // because all textures are based on current m_renderer

    fflassert(data);
    fflassert(size > 0);

    if(!m_renderer){
        return nullptr;
    }

    SDL_RWops   * rwOpsPtr = nullptr;
    SDL_Surface *  surfPtr = nullptr;
    SDL_Texture *   texPtr = nullptr;

    if((rwOpsPtr = SDL_RWFromConstMem(data, size))){
        if((surfPtr = IMG_LoadPNG_RW(rwOpsPtr))){
            texPtr = SDL_CreateTextureFromSurface(m_renderer, surfPtr);
        }
    }

    if(rwOpsPtr){
        SDL_FreeRW(rwOpsPtr);
    }

    if(surfPtr){
        SDL_FreeSurface(surfPtr);
    }
    return texPtr;
}

void SDLDevice::drawTexture(SDL_Texture *texPtr,
        int dstX, int dstY,
        int dstW, int dstH,
        int srcX, int srcY,
        int srcW, int srcH)
{
    if(texPtr){
        SDL_Rect src;
        SDL_Rect dst;

        src.x = srcX;
        src.y = srcY;
        src.w = srcW;
        src.h = srcH;

        dst.x = dstX;
        dst.y = dstY;
        dst.w = dstW;
        dst.h = dstH;

        SDL_RenderCopy(m_renderer, texPtr, &src, &dst);
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
                case DIR_UPRIGHT  : return {anchorX - texW    , anchorY           };
                case DIR_RIGHT    : return {anchorX - texW    , anchorY - texH / 2};
                case DIR_DOWNRIGHT: return {anchorX - texW    , anchorY - texH    };
                case DIR_DOWN     : return {anchorX - texW / 2, anchorY - texH    };
                case DIR_DOWNLEFT : return {anchorX           , anchorY - texH    };
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

    SDL_RWops * rwOpsPtr = nullptr;
    TTF_Font  *   ttfPtr = nullptr;

    if((rwOpsPtr = SDL_RWFromConstMem(data, size))){
        ttfPtr = TTF_OpenFontRW(rwOpsPtr, 1, fontPtSize);
    }

    // TODO
    // I checked the SDL_ttf.c
    // seems the TTF_OpenFontRW() will directly assign the SDL_RWops pointer
    // to inside pointer, and also the freeMark
    //
    // so here we provide SDL_RWops and freeMark
    // the TTF_Font struct will take control of free/assess the SDL_RWops
    // and we can't free it before destroy TTF_Font

    // so, don't use this code before the resource allocated by SDL_RWops
    // is done. WTF

    // if(rwOpsPtr){
    //     SDL_FreeRW(rwOpsPtr);
    // }

    return ttfPtr;
}

void SDLDevice::createInitViewWindow()
{
    if(m_renderer){
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if(m_window){
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    int windowW = 800;
    int windowH = 600;
    {
        SDL_DisplayMode desktop;
        if(!SDL_GetDesktopDisplayMode(0, &desktop)){
            windowW = std::min<int>(windowW , desktop.w);
            windowH = std::min<int>(windowH , desktop.h);
        }
    }

    m_window = SDL_CreateWindow("MIR2X-V0.1-LOADING", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowW, windowH, SDL_WINDOW_BORDERLESS);
    if(!m_window){
        throw fflerror("failed to create SDL window handler: %s", SDL_GetError());
    }

    SDL_ShowWindow(m_window);
    SDL_RaiseWindow(m_window);
    SDL_SetWindowResizable(m_window, SDL_FALSE);

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if(!m_renderer){
        SDL_DestroyWindow(m_window);
        throw fflerror("failed to create SDL renderer: %s", SDL_GetError());
    }

    setWindowIcon();

    if(SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255)){
        throw fflerror("set renderer draw color failed: %s", SDL_GetError());
    }

    if(SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND)){
        throw fflerror("set renderer blend mode failed: %s", SDL_GetError());
    }
}

void SDLDevice::createMainWindow()
{
    // need to clean the window resource
    // and abort if window allocation failed
    // InitView will allocate window before main window

    if(m_renderer){
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if(m_window){
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    const auto winFlag = []() -> Uint32
    {
        switch(g_clientArgParser->screenMode){
            case  1: return SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP;
            case  2: return SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN;
            default: return SDL_WINDOW_RESIZABLE;
        }
    }();

    m_window = SDL_CreateWindow("MIR2X-V0.1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SYS_WINDOW_MIN_W, SYS_WINDOW_MIN_H, winFlag);
    fflassert(m_window);

    SDL_SetWindowMinimumSize(m_window, SYS_WINDOW_MIN_W, SYS_WINDOW_MIN_H);
    m_renderer = SDL_CreateRenderer(m_window, -1, 0);

    if(!m_renderer){
        SDL_DestroyWindow(m_window);
        throw fflerror("failed to create SDL renderer: %s", SDL_GetError());
    }

    setWindowIcon();

    if(SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0)){
        throw fflerror("set renderer draw color failed: %s", SDL_GetError());
    }

    if(SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND)){
        throw fflerror("set renderer blend mode failed: %s", SDL_GetError());
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
        SDL_RendererFlip flip)
{
    // how SDL_RenderCopyEx works
    //
    //   1. crop image
    //   2. flip the small cropped image, not the full image
    //   3. draw cropped and flipped image to given destionation
    //   4. rotate using offset to destionation, positive means clockwise

    if(texPtr){
        SDL_Rect src {srcX, srcY, srcW, srcH};
        SDL_Rect dst {dstX, dstY, dstW, dstH};
        SDL_Point center {centerDstOffX, centerDstOffY};
        const double angle = 1.00 * (rotateDegree % 360);
        if(SDL_RenderCopyEx(m_renderer, texPtr, &src, &dst, angle, &center, flip)){
            throw fflerror("SDL_RenderCopyEx(%p) failed: %s", to_cvptr(m_renderer), SDL_GetError());
        }

        if(g_clientArgParser->debugDrawTexture){
            const double radian = -1.0 * angle / 180.0 * mathf::pi;

            // not very accurate, because rotation center is not at pixel
            // take following 4-pixel-image as example
            //
            //     +-+-+
            //     |a|b|
            //     +-*-+
            //     |d|c|
            //     +-+-+
            //
            // drawTextureEx(tex,
            //          0, 0, 2, 2,
            //          0, 0, 2, 2,
            //
            //          1,
            //          1,
            //          90,
            //
            //          SDL_FLIP_NONE);
            //
            // above call actually means rotate at the star (*) position
            // not at pixel (1, 1)

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
    // TODO
    // seems we can only use SDL_PIXELFORMAT_RGBA8888
    // tried SDL_PIXELFORMAT_RGBA32 but gives wrong pixel format

    fflassert(data);
    fflassert(w > 0);
    fflassert(h > 0);

    if(auto texPtr = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, w, h)){
        if(!SDL_UpdateTexture(texPtr, 0, data, w * 4) && !SDL_SetTextureBlendMode(texPtr, SDL_BLENDMODE_BLEND)){
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

    const static Rawbuf s_defaultTTFData
    {
        #include "monaco.rawbuf"
    };

    if(auto ttfPtr = createTTF(s_defaultTTFData.data(), s_defaultTTFData.size(), fontSize); ttfPtr){
        return m_fontList[fontSize] = ttfPtr;
    }
    throw fflerror("can't build default ttf with point: %llu", to_llu(fontSize));
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
        throw fflerror("invalid registered cover: r = %d, angle = %d", r, angle);
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

            if(curr_r2 < r * r && mathf::bound<int>(curr_angle, 0, 360) <= angle){
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
    throw fflerror("failed to create texture: r = %d, angle = %d", r, angle);
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

    if(SDL_GetRenderDrawColor(getRenderer(), &r, &g, &b, &a)){
        throw fflerror("get renderer draw color failed: %s", SDL_GetError());
    }

    if(a == 0){
        return;
    }

    if(roundedBoxRGBA(getRenderer(), argX, argY, argX + argW - 1, argY + argH - 1, argRad, r, g, b, a)){
        throw fflerror("roundedRectangleRGBA() failed");
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
        aacircleRGBA(m_renderer, argX, argY, argRad, colorf::R(color), colorf::G(color), colorf::B(color), colorf::A(color));
    }
}

void SDLDevice::fillCircle(uint32_t color, int argX, int argY, int argRad)
{
    fflassert(argRad >= 0, argRad);
    if(colorf::A(color)){
        filledCircleRGBA(m_renderer, argX, argY, argRad, colorf::R(color), colorf::G(color), colorf::B(color), colorf::A(color));
    }
}

void SDLDevice::drawTriangle(int argX1, int argY1, int argX2, int argY2, int argX3, int argY3)
{
    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
    Uint8 a = 0;

    if(SDL_GetRenderDrawColor(getRenderer(), &r, &g, &b, &a)){
        throw fflerror("get renderer draw color failed: %s", SDL_GetError());
    }

    if(a == 0){
        return;
    }

    if(aatrigonRGBA(getRenderer(), argX1, argY1, argX2, argY2, argX3, argY3, r, g, b, a)){
        throw fflerror("aatrigonRGBA() failed");
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

    if(SDL_GetRenderDrawColor(getRenderer(), &r, &g, &b, &a)){
        throw fflerror("get renderer draw color failed: %s", SDL_GetError());
    }

    if(a == 0){
        return;
    }

    if(filledTrigonRGBA(getRenderer(), argX1, argY1, argX2, argY2, argX3, argY3, r, g, b, a)){
        throw fflerror("filledTrigonRGBA() failed");
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
    // hack for roundedRectangleRGBA
    // looks sdl2_gfx has 1 pixel location bug

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

    if(SDL_GetRenderDrawColor(getRenderer(), &r, &g, &b, &a)){
        throw fflerror("get renderer draw color failed: %s", SDL_GetError());
    }

    if(a == 0){
        return;
    }

    if(roundedRectangleRGBA(getRenderer(), argX, argY, argX + argW - 1, argY + argH - 1, argRad, r, g, b, a)){
        throw fflerror("roundedRectangleRGBA() failed");
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
            SDL_RenderDrawPoint(getRenderer(), x + i * (length > 0 ? 1 : -1), y);
        }
    }
}

void SDLDevice::drawVLineFading(uint32_t startColor, uint32_t endColor, int x, int y, int length)
{
    if(length){
        SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
        for(int i = 0; i < std::abs(length); ++i){
            SDLDeviceHelper::EnableRenderColor enableColor(colorf::fadeRGBA(startColor, endColor, i * 1.0f / std::abs(length)), this);
            SDL_RenderDrawPoint(getRenderer(), x, y + i * (length > 0 ? 1 : -1));
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

    // define the direction current x/y increments
    //
    //  +------------ start point
    //  |
    //  v   2
    //  <-------A
    //  |       |
    // 3|       |1
    //  |   0   |
    //  v------->
    //

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
        SDL_RenderDrawPoint(getRenderer(), currX, currY);
        std::tie(currX, currY, currDir) = fnNextPoint(currX, currY, currDir);
    }
}

void SDLDevice::drawString(uint32_t color, int x, int y, const char *s)
{
    if(stringRGBA(m_renderer, x, y, s, colorf::R(color), colorf::G(color), colorf::B(color), colorf::A(color))){
        throw fflerror("failed to draw 8x8 string: %s", s);
    }
}

void SDLDevice::setWindowSize(int w, int h)
{
    fflassert(w >= 400, w, h);
    fflassert(h >= 400, w, h);

    SDL_SetWindowSize(m_window, w, h);
}

void SDLDevice::stopBGM()
{
    if(g_clientArgParser->disableAudio){
        return;
    }
    Mix_HaltMusic();
}

void SDLDevice::setBGMVolume(float volume)
{
    if(g_clientArgParser->disableAudio){
        return;
    }
    Mix_VolumeMusic(std::lround(mathf::bound<float>(volume, 0.0f, 1.0f) * SDL_MIX_MAXVOLUME));
}

void SDLDevice::playBGM(Mix_Music *music, size_t repeats)
{
    if(g_clientArgParser->disableAudio){
        return;
    }

    if(music){
        if(Mix_PlayMusic(music, (repeats == 0) ? -1 : (to_d(repeats) - 1))){
            throw fflerror("failed to play music: %s", Mix_GetError());
        }
    }
}

std::shared_ptr<SDLSoundEffectChannel> SDLDevice::playSoundEffect(std::shared_ptr<SoundEffectHandle> handle, int distance, int angle, size_t repeats)
{
    if(g_clientArgParser->disableAudio){
        return {};
    }

    if(!handle){
        return {};
    }

    if(!handle->chunk){
        return {};
    }

    if(distance < 0){
        return {};
    }

    int pickedChannel = -1;
    scoped_alloc::svobuf_wrapper<int64_t, 64> idleChannelList; // use int64_t because ASAN reports error: alignment < 8
    {
        // clean pending channel
        // can't delete channel and handle in Mix_ChannelFinished()

        // for channels in m_freeChannelList, their are not playing
        // but there can still be a SDLSoundEffectChannel hooked to it, and can control it
        // shall not allocate it to new sound effect before this hooked SDLSoundEffectChannel get released

        const std::lock_guard<std::mutex> lockGuard(m_freeChannelLock);
        for(const auto channel: m_freeChannelList){
            if(auto p = m_channelStateList.find(channel); (p != m_channelStateList.end()) && p->second.hooked){
                continue;
            }
            else{
                idleChannelList.c.push_back(channel);
            }
        }

        if(idleChannelList.c.empty()){
            return {};
        }

        pickedChannel = to_d(idleChannelList.c.back());
        m_freeChannelList.erase(pickedChannel);
    }

    for(const auto idleChannel: idleChannelList.c){
        m_channelStateList.erase(idleChannel);
    }

    fflassert(pickedChannel >= 0, pickedChannel);
    m_channelStateList.try_emplace(pickedChannel, true, handle);

    // Mix_HaltChannel() does nothing if channel has done play
    // otherwise halt the channel and call SDLDevice::recycleSoundEffectChannel()

    Mix_HaltChannel(pickedChannel);
    Mix_SetPosition(pickedChannel, ((angle % 360) + 360) % 360, distance);
    Mix_PlayChannel(pickedChannel, handle->chunk, (repeats == 0) ? -1 : (to_d(repeats) - 1));

    // return shared_ptr that hooks to m_channelStateList[pickedChannel].hooked
    // p->dtor() or p->halt() shall get called to unhook the flag, otherwise this channel gets hold on forever
    return std::shared_ptr<SDLSoundEffectChannel>(new SDLSoundEffectChannel
    {
        this,
        pickedChannel,
    });
}

void SDLDevice::stopSoundEffect()
{
    if(g_clientArgParser->disableAudio){
        return;
    }
    Mix_HaltChannel(-1);
}

void SDLDevice::setSoundEffectVolume(float volume)
{
    if(g_clientArgParser->disableAudio){
        return;
    }

    // use post channel to configure the volume
    // each channel has own volume and may change during playing, like firewall sound changes when MyHero moves

    if(!Mix_SetDistance(MIX_CHANNEL_POST, std::lround((1.0f - mathf::bound<float>(volume, 0.0f, 1.0f)) * 255))){
        throw fflerror("failed to setup sound effect volume: %s", Mix_GetError());
    }
}

void SDLDevice::recycleSoundEffectChannel(int channel)
{
    if(g_clientArgParser->disableAudio){
        throw fflerror("sound effect callback is triggered with audio disabled");
    }

    // only put the channel to free list
    // don't call handle.reset() here which may call into Mix_Funcs and is forbidden

    // handle reset is done in next time when channel allocated for new playing
    // this may cause resouce free delay

    // don't need to lock here even it's called other than main thread
    // SDL2 mixer calls SDL_LockAudio()/SDL_UnlockAudio()

    const std::lock_guard<std::mutex> lockGuard(g_sdlDevice->m_freeChannelLock);
    g_sdlDevice->m_freeChannelList.insert(channel);
}
