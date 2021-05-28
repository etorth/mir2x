/*
 * =====================================================================================
 *
 *       Filename: sdldevice.cpp
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

#include <cinttypes>
#include <system_error>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "log.hpp"
#include "mathf.hpp"
#include "totype.hpp"
#include "rawbuf.hpp"
#include "xmlconf.hpp"
#include "fflerror.hpp"
#include "sdldevice.hpp"
#include "clientargparser.hpp"

extern Log *g_log;
extern XMLConf *g_XMLConf;
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

SDLDeviceHelper::SDLEventPLoc SDLDeviceHelper::getEventPLoc(const SDL_Event &event)
{
    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                return {event.motion.x, event.motion.y};
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                return {event.button.x, event.button.y};
            }
        default:
            {
                return {-1, -1};
            }
    }
}

std::tuple<int, int> SDLDeviceHelper::getTextureSize(SDL_Texture *texture)
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

int SDLDeviceHelper::getTextureWidth(SDL_Texture *texture)
{
    return std::get<0>(getTextureSize(texture));
}

int SDLDeviceHelper::getTextureHeight(SDL_Texture *texture)
{
    return std::get<1>(getTextureSize(texture));
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
}

SDLDevice::~SDLDevice()
{
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

    SDL_StopTextInput();
    SDL_Quit();
}

void SDLDevice::SetWindowIcon()
{
    Uint16 pRawData[16 * 16]
    {
        #include "winicon.inc"
    };

    if(auto pstSurface = SDL_CreateRGBSurfaceFrom(pRawData, 16, 16, 16, 16*2, 0x0f00, 0x00f0, 0x000f, 0xf000)){
        SDL_SetWindowIcon(m_window, pstSurface);
        SDL_FreeSurface(pstSurface);
    }
}

void SDLDevice::toggleWindowFullscreen()
{
    fflassert(m_window);
    const auto winFlag = SDL_GetWindowFlags(m_window);

    if(winFlag & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)){
        SDL_SetWindowFullscreen(m_window, 0);
    }
    else{
        SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
    }
}

SDL_Texture *SDLDevice::CreateTexture(const uint8_t *pMem, size_t nSize)
{
    // if it's changed
    // all the texture need to be re-load

    // currently it doesn't support dynamic set of context
    // because all textures are based on current m_renderer

    SDL_RWops   *pstRWops   = nullptr;
    SDL_Surface *pstSurface = nullptr;
    SDL_Texture *pstTexture = nullptr;

    if(pMem && nSize){
        pstRWops = SDL_RWFromConstMem((const void *)(pMem), nSize);
        if(pstRWops){
            pstSurface = IMG_LoadPNG_RW(pstRWops);
            if(pstSurface){
                if(m_renderer){
                    pstTexture = SDL_CreateTextureFromSurface(m_renderer, pstSurface);
                }
            }
        }
    }

    // TODO
    // not understand well for SDL_FreeRW()
    // since the creation is done we can free it?
    if(pstRWops){
        SDL_FreeRW(pstRWops);
    }

    if(pstSurface){
        SDL_FreeSurface(pstSurface);
    }
    return pstTexture;
}

void SDLDevice::drawTexture(SDL_Texture *pstTexture,
        int nDstX, int nDstY,
        int nDstW, int nDstH,
        int nSrcX, int nSrcY,
        int nSrcW, int nSrcH)
{
    if(pstTexture){
        SDL_Rect stSrc;
        SDL_Rect stDst;

        stSrc.x = nSrcX;
        stSrc.y = nSrcY;
        stSrc.w = nSrcW;
        stSrc.h = nSrcH;

        stDst.x = nDstX;
        stDst.y = nDstY;
        stDst.w = nDstW;
        stDst.h = nDstH;

        SDL_RenderCopy(m_renderer, pstTexture, &stSrc, &stDst);
        if(g_clientArgParser->debugDrawTexture){
            drawRectangle(colorf::BLUE + 128, nDstX, nDstY, nDstW, nDstH);
        }
    }
}

void SDLDevice::drawTexture(SDL_Texture *pstTexture,
        int nDstX, int nDstY,
        int nSrcX, int nSrcY,
        int nSrcW, int nSrcH)
{
    drawTexture(pstTexture, nDstX, nDstY, nSrcW, nSrcH, nSrcX, nSrcY, nSrcW, nSrcH);
}

void SDLDevice::drawTexture(SDL_Texture *pstTexture, int nX, int nY)
{
    if(pstTexture){
        int nW, nH;
        if(!SDL_QueryTexture(pstTexture, nullptr, nullptr, &nW, &nH)){
            drawTexture(pstTexture, nX, nY, 0, 0, nW, nH);
        }
    }
}

TTF_Font *SDLDevice::createTTF(const uint8_t *pMem, size_t nSize, uint8_t nFontPointSize)
{
    SDL_RWops *pstRWops = nullptr;
    TTF_Font  *pstTTont = nullptr;

    if(pMem && nSize && nFontPointSize){
        pstRWops = SDL_RWFromConstMem((const void *)(pMem), nSize);
        if(pstRWops){
            pstTTont = TTF_OpenFontRW(pstRWops, 1, nFontPointSize);
        }
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

    // if(pstRWops){
    //     SDL_FreeRW(pstRWops);
    // }

    return pstTTont;
}

void SDLDevice::CreateInitViewWindow()
{
    if(m_renderer){
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if(m_window){
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    int nWindowW = 388;
    int nWindowH = 160;
    {
        SDL_DisplayMode stDesktop;
        if(!SDL_GetDesktopDisplayMode(0, &stDesktop)){
            nWindowW = std::min<int>(nWindowW , stDesktop.w);
            nWindowH = std::min<int>(nWindowH , stDesktop.h);
        }
    }

    m_window = SDL_CreateWindow("MIR2X-V0.1-LOADING", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, nWindowW, nWindowH, SDL_WINDOW_BORDERLESS);
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

    SetWindowIcon();

    if(SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255)){
        throw fflerror("set renderer draw color failed: %s", SDL_GetError());
    }

    if(SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND)){
        throw fflerror("set renderer blend mode failed: %s", SDL_GetError());
    }
}

void SDLDevice::CreateMainWindow()
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
        switch(g_XMLConf->to_d("Root/Window/ScreenMode").value_or(-1)){
            case  1: return SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP;
            case  2: return SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN;
            default: return SDL_WINDOW_RESIZABLE;
        }
    }();

    m_window = SDL_CreateWindow("MIR2X-V0.1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, winFlag);
    fflassert(m_window);

    SDL_SetWindowMinimumSize(m_window, 800, 600);
    m_renderer = SDL_CreateRenderer(m_window, -1, 0);

    if(!m_renderer){
        SDL_DestroyWindow(m_window);
        throw fflerror("failed to create SDL renderer: %s", SDL_GetError());
    }

    SetWindowIcon();

    if(SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0)){
        throw fflerror("set renderer draw color failed: %s", SDL_GetError());
    }

    if(SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND)){
        throw fflerror("set renderer blend mode failed: %s", SDL_GetError());
    }

    SDL_StartTextInput();
}

void SDLDevice::drawTextureEx(SDL_Texture *pTexture,
        int nSrcX, int nSrcY, int nSrcW, int nSrcH,
        int nDstX, int nDstY, int nDstW, int nDstH,
        int centerSrcX,
        int centerDstX,
        int nRotateDegree)
{
    if(pTexture){
        SDL_Rect stSrc {nSrcX, nSrcY, nSrcW, nSrcH};
        SDL_Rect stDst {nDstX, nDstY, nDstW, nDstH};

        const double fAngle = 1.00 * (nRotateDegree % 360);
        SDL_Point stCenter {centerSrcX, centerDstX};
        SDL_RenderCopyEx(m_renderer, pTexture, &stSrc, &stDst, fAngle, &stCenter, SDL_FLIP_NONE);
    }
}

SDL_Texture *SDLDevice::createTexture(const uint32_t *buf, int w, int h)
{
    // TODO
    // seems we can only use SDL_PIXELFORMAT_RGBA8888
    // tried SDL_PIXELFORMAT_RGBA32 but gives wrong pixel format
    //
    if(auto ptex = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, w, h)){
        if(!SDL_UpdateTexture(ptex, 0, buf, w * 4) && !SDL_SetTextureBlendMode(ptex, SDL_BLENDMODE_BLEND)){
            return ptex;
        }
        SDL_DestroyTexture(ptex);
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
                return 255 - std::min<uint8_t>(255, std::lround(255.0 * curr_r2 / (r * r)));
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

    if(auto texPtr = createTexture(buf.data(), w, h)){
        return m_cover[key] = texPtr;
    }
    throw fflerror("failed to create texture: r = %d, angle = %d", r, angle);
}

void SDLDevice::fillRectangle(int nX, int nY, int nW, int nH, int nRad)
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

    if(roundedBoxRGBA(getRenderer(), nX, nY, nX + nW, nY + nH, nRad, r, g, b, a)){
        throw fflerror("roundedRectangleRGBA() failed");
    }
}

void SDLDevice::fillRectangle(uint32_t nRGBA, int nX, int nY, int nW, int nH, int nRad)
{
    if(colorf::A(nRGBA)){
        SDLDeviceHelper::EnableRenderColor stEnableColor(nRGBA, this);
        SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
        fillRectangle(nX, nY, nW, nH, nRad);
    }
}

void SDLDevice::drawRectangle(int nX, int nY, int nW, int nH, int nRad)
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

    if(roundedRectangleRGBA(getRenderer(), nX, nY, nX + nW, nY + nH, nRad, r, g, b, a)){
        throw fflerror("roundedRectangleRGBA() failed");
    }
}

void SDLDevice::drawRectangle(uint32_t color, int nX, int nY, int nW, int nH, int nRad)
{
    if(colorf::A(color)){
        SDLDeviceHelper::EnableRenderColor enableColor(color, this);
        SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
        drawRectangle(nX, nY, nW, nH, nRad);
    }
}

void SDLDevice::drawWidthRectangle(size_t frameLineWidth, int nX, int nY, int nW, int nH)
{
    if(!frameLineWidth){
        return;
    }

    if(frameLineWidth == 1){
        drawRectangle(nX, nY, nW, nH);
        return;
    }

    fillRectangle(nX, nY,                           nW, frameLineWidth);
    fillRectangle(nX, nY + nW - 2 * frameLineWidth, nW, frameLineWidth);

    fillRectangle(nX,                           nY + frameLineWidth, frameLineWidth, nH - 2 * frameLineWidth);
    fillRectangle(nX + nW - 2 * frameLineWidth, nY + frameLineWidth, frameLineWidth, nH - 2 * frameLineWidth);
}

void SDLDevice::drawWidthRectangle(uint32_t color, size_t frameLineWidth, int nX, int nY, int nW, int nH)
{
    SDLDeviceHelper::EnableRenderColor enableColor(color, this);
    SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
    drawWidthRectangle(frameLineWidth, nX, nY, nW, nH);
}

void SDLDevice::drawString(uint32_t color, int x, int y, const char *s)
{
    if(stringRGBA(m_renderer, x, y, s, colorf::R(color), colorf::G(color), colorf::B(color), colorf::A(color))){
        throw fflerror("failed to draw 8x8 string: %s", s);
    }
}
