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
#include <SDL2/SDL.h>
#include <system_error>
#include <SDL2/SDL_image.h>

#include "log.hpp"
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
    Uint16 pRawData[16 * 16] = {
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0AAB, 0X0789, 0X0BCC, 0X0EEE, 0X09AA, 0X099A, 0X0DDD,
        0X0FFF, 0X0EEE, 0X0899, 0X0FFF, 0X0FFF, 0X1FFF, 0X0DDE, 0X0DEE,
        0X0FFF, 0XABBC, 0XF779, 0X8CDD, 0X3FFF, 0X9BBC, 0XAAAB, 0X6FFF,
        0X0FFF, 0X3FFF, 0XBAAB, 0X0FFF, 0X0FFF, 0X6689, 0X6FFF, 0X0DEE,
        0XE678, 0XF134, 0X8ABB, 0XF235, 0XF678, 0XF013, 0XF568, 0XF001,
        0XD889, 0X7ABC, 0XF001, 0X0FFF, 0X0FFF, 0X0BCC, 0X9124, 0X5FFF,
        0XF124, 0XF356, 0X3EEE, 0X0FFF, 0X7BBC, 0XF124, 0X0789, 0X2FFF,
        0XF002, 0XD789, 0XF024, 0X0FFF, 0X0FFF, 0X0002, 0X0134, 0XD79A,
        0X1FFF, 0XF023, 0XF000, 0XF124, 0XC99A, 0XF024, 0X0567, 0X0FFF,
        0XF002, 0XE678, 0XF013, 0X0FFF, 0X0DDD, 0X0FFF, 0X0FFF, 0XB689,
        0X8ABB, 0X0FFF, 0X0FFF, 0XF001, 0XF235, 0XF013, 0X0FFF, 0XD789,
        0XF002, 0X9899, 0XF001, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0XE789,
        0XF023, 0XF000, 0XF001, 0XE456, 0X8BCC, 0XF013, 0XF002, 0XF012,
        0X1767, 0X5AAA, 0XF013, 0XF001, 0XF000, 0X0FFF, 0X7FFF, 0XF124,
        0X0FFF, 0X089A, 0X0578, 0X0FFF, 0X089A, 0X0013, 0X0245, 0X0EFF,
        0X0223, 0X0DDE, 0X0135, 0X0789, 0X0DDD, 0XBBBC, 0XF346, 0X0467,
        0X0FFF, 0X4EEE, 0X3DDD, 0X0EDD, 0X0DEE, 0X0FFF, 0X0FFF, 0X0DEE,
        0X0DEF, 0X08AB, 0X0FFF, 0X7FFF, 0XFABC, 0XF356, 0X0457, 0X0467,
        0X0FFF, 0X0BCD, 0X4BDE, 0X9BCC, 0X8DEE, 0X8EFF, 0X8FFF, 0X9FFF,
        0XADEE, 0XECCD, 0XF689, 0XC357, 0X2356, 0X0356, 0X0467, 0X0467,
        0X0FFF, 0X0CCD, 0X0BDD, 0X0CDD, 0X0AAA, 0X2234, 0X4135, 0X4346,
        0X5356, 0X2246, 0X0346, 0X0356, 0X0467, 0X0356, 0X0467, 0X0467,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF};

    if(auto pstSurface = SDL_CreateRGBSurfaceFrom(pRawData, 16, 16, 16, 16*2, 0x0f00, 0x00f0, 0x000f, 0xf000)){
        SDL_SetWindowIcon(m_window, pstSurface);
        SDL_FreeSurface(pstSurface);
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
        SDL_Rect stSrc {nSrcX, nSrcY, nSrcW, nSrcH};
        SDL_Rect stDst {nDstX, nDstY, nDstW, nDstH};
        SDL_RenderCopy(m_renderer, pstTexture, &stSrc, &stDst);
        if(g_clientArgParser->debugdrawTexture){
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

    Uint32 nFlags   = 0;
    int    nWindowW = 0;
    int    nWindowH = 0;

    int nScreenMode = 0;
    if(g_XMLConf->NodeAtoi("Root/Window/ScreenMode", &nScreenMode, 0)){
        g_log->addLog(LOGTYPE_INFO, "screen mode by configuration file: %d", nScreenMode);
    }else{
        g_log->addLog(LOGTYPE_WARNING, "Failed to select screen mode by configuration file.");
    }

    switch(nScreenMode){
        case 0:
            break;
        case 1:
            nFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            break;
        case 2:
            nFlags |= SDL_WINDOW_FULLSCREEN;
            break;
        default:
            break;
    }

    if(true
            && g_XMLConf->NodeAtoi("Root/Window/W", &nWindowW, 800)
            && g_XMLConf->NodeAtoi("Root/Window/H", &nWindowH, 600)){
        g_log->addLog(LOGTYPE_INFO, "window size by configuration file: %d x %d", nWindowW, nWindowH);
    }

    else{
        g_log->addLog(LOGTYPE_INFO, "Use default window size.");
        nWindowW = 800;
        nWindowH = 600;
    }

    nWindowW = std::max<int>(nWindowW, 800);
    nWindowH = std::max<int>(nWindowH, 600);

    SDL_DisplayMode stDesktop;
    if(!SDL_GetDesktopDisplayMode(0, &stDesktop)){
        nWindowW = std::min(nWindowW, stDesktop.w);
        nWindowH = std::min(nWindowH, stDesktop.h);
    }

    if(nWindowW < 800 || nWindowH < 600){
        throw fflerror("window size is too small: width = %d, height = %d", nWindowW, nWindowH);
    }

    nFlags |= SDL_WINDOW_RESIZABLE;
    m_window = SDL_CreateWindow("MIR2X-V0.1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, nWindowW, nWindowH, nFlags);
    if(!m_window){
        throw fflerror("failed to create SDL window handler: %s", SDL_GetError());
    }

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
        int nCenterDstX,
        int nCenterDstY,
        int nRotateDegree)
{
    if(pTexture){
        SDL_Rect stSrc {nSrcX, nSrcY, nSrcW, nSrcH};
        SDL_Rect stDst {nDstX, nDstY, nDstW, nDstH};

        double fAngle = 1.00 * (nRotateDegree % 360);
        SDL_Point stCenter {nCenterDstX, nCenterDstY};

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

TTF_Font *SDLDevice::DefaultTTF(uint8_t fontSize)
{
    const static Rawbuf s_defaultTTFData
    {
        #include "monaco.rawbuf"
    };

    if(auto p = m_fontList.find(fontSize); p != m_fontList.end()){
        return p->second;
    }

    if(auto ttfPtr = createTTF(s_defaultTTFData.data(), s_defaultTTFData.size(), fontSize); ttfPtr){
        return m_fontList[fontSize] = ttfPtr;
    }
    throw fflerror("can't build default ttf with point: %llu", to_llu(fontSize));
}

SDL_Texture *SDLDevice::getCover(int r)
{
    if(r <= 0){
        throw fflerror("invalid argument: radius = %d", r);
    }

    if(auto p = m_cover.find(r); p != m_cover.end()){
        if(p->second){
            return p->second;
        }
        throw fflerror("invalid registered cover: radius = %d", r);
    }

    const int w = r * 2 - 1;
    const int h = r * 2 - 1;

    std::vector<uint32_t> buf(w * h);
    for(int y = 0; y < h; ++y){
        for(int x = 0; x < w; ++x){
            const int currR2 = (x - r + 1) * (x - r + 1) + (y - r + 1) * (y - r + 1);
            const uint8_t alpha = [currR2, r]() -> uint8_t
            {
                if(g_clientArgParser->debugAlphaCover){
                    return 255;
                }
                return 255 - std::min<uint8_t>(255, std::lround(255.0 * currR2 / (r * r)));
            }();
            if(currR2 < r * r){
                buf[x + y * w] = colorf::RGBA(0XFF, 0XFF, 0XFF, alpha);
            }
            else{
                buf[x + y * w] = colorf::RGBA(0, 0, 0, 0);
            }
        }
    }

    if(auto texPtr = createTexture(buf.data(), w, h)){
        return m_cover[r] = texPtr;
    }
    throw fflerror("creature texture failed: radius = %d", r);
}

void SDLDevice::fillRectangle(int nX, int nY, int nW, int nH)
{
    SDL_Rect stRect;
    stRect.x = nX;
    stRect.y = nY;
    stRect.w = nW;
    stRect.h = nH;

    SDL_RenderFillRect(m_renderer, &stRect);
}

void SDLDevice::fillRectangle(uint32_t nRGBA, int nX, int nY, int nW, int nH)
{
    if(colorf::A(nRGBA)){
        SDLDeviceHelper::EnableRenderColor stEnableColor(nRGBA, this);
        SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
        fillRectangle(nX, nY, nW, nH);
    }
}

void SDLDevice::drawRectangle(int nX, int nY, int nW, int nH)
{
    SDL_Rect rect;
    rect.x = nX;
    rect.y = nY;
    rect.w = nW;
    rect.h = nH;

    SDL_RenderDrawRect(m_renderer, &rect);
    SDL_RenderDrawPoint(m_renderer, rect.x + rect.w - 1, rect.y + rect.h - 1);
}

void SDLDevice::drawRectangle(uint32_t color, int nX, int nY, int nW, int nH)
{
    if(colorf::A(color)){
        SDLDeviceHelper::EnableRenderColor enableColor(color, this);
        SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND, this);
        drawRectangle(nX, nY, nW, nH);
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
