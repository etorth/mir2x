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

#include <SDL2/SDL.h>
#include <system_error>
#include <SDL2/SDL_image.h>

#include "log.hpp"
#include "xmlconf.hpp"
#include "sdldevice.hpp"
#include "condcheck.hpp"

SDLDevice::SDLDevice()
    : m_Window(nullptr)
    , m_Renderer(nullptr)
    , m_ColorStack()
    , m_BlendModeStack()
    , m_WindowW(0)
    , m_WindowH(0)
{
    extern SDLDevice *g_SDLDevice;
    if(g_SDLDevice){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Multiple initialization for SDLDevice");
    }

    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS)){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Initialization failed for SDL2: %s", SDL_GetError());
    }

    if(TTF_Init()){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Initialization failed for SDL2 TTF: %s", TTF_GetError());
    }

    if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Initialization failed for SDL2 IMG: %s", IMG_GetError());
    }
}

SDLDevice::~SDLDevice()
{
    if(m_Window){
        SDL_DestroyWindow(m_Window);
    }

    if(m_Renderer){
        SDL_DestroyRenderer(m_Renderer);
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
        SDL_SetWindowIcon(m_Window, pstSurface);
        SDL_FreeSurface(pstSurface);
    }
}

SDL_Texture *SDLDevice::CreateTexture(const uint8_t *pMem, size_t nSize)
{
    // if it's changed
    // all the texture need to be re-load

    // currently it doesn't support dynamic set of context
    // because all textures are based on current m_Renderer

    SDL_RWops   *pstRWops   = nullptr;
    SDL_Surface *pstSurface = nullptr;
    SDL_Texture *pstTexture = nullptr;

    if(pMem && nSize){
        pstRWops = SDL_RWFromConstMem((const void *)(pMem), nSize);
        if(pstRWops){
            pstSurface = IMG_LoadPNG_RW(pstRWops);
            if(pstSurface){
                if(m_Renderer){
                    pstTexture = SDL_CreateTextureFromSurface(m_Renderer, pstSurface);
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

void SDLDevice::DrawTexture(SDL_Texture *pstTexture,
        int nDstX, int nDstY,
        int nDstW, int nDstH,
        int nSrcX, int nSrcY,
        int nSrcW, int nSrcH)
{
    if(pstTexture){
        SDL_Rect stSrc {nSrcX, nSrcY, nSrcW, nSrcH};
        SDL_Rect stDst {nDstX, nDstY, nDstW, nDstH};
        SDL_RenderCopy(m_Renderer, pstTexture, &stSrc, &stDst);
    }
}

void SDLDevice::DrawTexture(SDL_Texture *pstTexture,
        int nDstX, int nDstY,
        int nSrcX, int nSrcY,
        int nSrcW, int nSrcH)
{
    if(pstTexture){
        SDL_Rect stSrc {nSrcX, nSrcY, nSrcW, nSrcH};
        SDL_Rect stDst {nDstX, nDstY, nSrcW, nSrcH};
        SDL_RenderCopy(m_Renderer, pstTexture, &stSrc, &stDst);
    }
}

void SDLDevice::DrawTexture(SDL_Texture *pstTexture, int nX, int nY)
{
    if(pstTexture){
        int nW, nH;
        if(!SDL_QueryTexture(pstTexture, nullptr, nullptr, &nW, &nH)){
            DrawTexture(pstTexture, nX, nY, 0, 0, nW, nH);
        }
    }
}

TTF_Font *SDLDevice::CreateTTF(const uint8_t *pMem, size_t nSize, uint8_t nFontPointSize)
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

void SDLDevice::PushColor(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
{
    uint32_t nARGB = ColorFunc::ARGB(nA, nR, nG, nB);
    if(m_ColorStack.empty() || nARGB != m_ColorStack.back()[0]){
        SetColor(nR, nG, nB, nA);
        m_ColorStack.push_back({nARGB, 1});
    }else{
        ++m_ColorStack.back()[1];
    }
}

void SDLDevice::PopColor()
{
    if(m_ColorStack.empty()){
        PushColor(0, 0, 0, 0);
    }else{
        condcheck(m_ColorStack.back()[1]);
        if(m_ColorStack.back()[1] == 1){
            m_ColorStack.pop_back();
            if(m_ColorStack.empty()){
                PushColor(0, 0, 0, 0);
            }else{
                SDL_Color stColor = ColorFunc::RGBA2Color(ColorFunc::ARGB2RGBA((m_ColorStack.back()[0])));
                SetColor(stColor.r, stColor.g, stColor.b, stColor.a);
            }
        }else{
            --m_ColorStack.back()[1];
        }
    }
}

void SDLDevice::PushBlendMode(SDL_BlendMode stMode)
{
    if(m_BlendModeStack.empty() || stMode != m_BlendModeStack.back().first){
        SDL_SetRenderDrawBlendMode(m_Renderer, stMode);
        m_BlendModeStack.push_back({stMode, 1});
    }else{
        ++m_BlendModeStack.back().second;
    }
}

void SDLDevice::PopBlendMode()
{
    if(m_BlendModeStack.empty()){
        PushBlendMode(SDL_BLENDMODE_NONE);
    }else{
        condcheck(m_BlendModeStack.back().second);
        if(m_BlendModeStack.back().second == 1){
            m_BlendModeStack.pop_back();
            if(m_BlendModeStack.empty()){
                PushBlendMode(SDL_BLENDMODE_NONE);
            }else{
                SDL_SetRenderDrawBlendMode(m_Renderer, m_BlendModeStack.back().first);
            }
        }else{
            --m_BlendModeStack.back().second;
        }
    }
}

void SDLDevice::CreateInitViewWindow()
{
    if(m_Renderer){
        SDL_DestroyRenderer(m_Renderer);
        m_Renderer = nullptr;
    }

    if(m_Window){
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
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

    m_Window = SDL_CreateWindow("MIR2X-V0.1-LOADING", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, nWindowW, nWindowH, SDL_WINDOW_BORDERLESS);
    if(!m_Window){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Failed to create SDL window handler: %s", SDL_GetError());
    }

    m_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
    if(!m_Renderer){
        SDL_DestroyWindow(m_Window);
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Failed to create SDL renderer: %s", SDL_GetError());
    }

    SetWindowIcon();
    PushColor(0, 0, 0, 0);
    PushBlendMode(SDL_BLENDMODE_NONE);
}

void SDLDevice::CreateMainWindow()
{
    // need to clean the window resource
    // and abort if window allocation failed
    // InitView will allocate window before main window

    if(m_Renderer){
        SDL_DestroyRenderer(m_Renderer);
        m_Renderer = nullptr;
    }

    if(m_Window){
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }

    Uint32 nFlags   = 0;
    int    nWindowW = 0;
    int    nWindowH = 0;

    extern Log     *g_Log;
    extern XMLConf *g_XMLConf;
    int nScreenMode = 0;
    if(g_XMLConf->NodeAtoi("Root/Window/ScreenMode", &nScreenMode, 0)){
        g_Log->AddLog(LOGTYPE_INFO, "screen mode by configuration file: %d", nScreenMode);
    }else{
        g_Log->AddLog(LOGTYPE_WARNING, "Failed to select screen mode by configuration file.");
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
        g_Log->AddLog(LOGTYPE_INFO, "window size by configuration file: %d x %d", nWindowW, nWindowH);
    }else{
        g_Log->AddLog(LOGTYPE_INFO, "Use default window size.");
        nWindowW = 800;
        nWindowH = 600;
    }

    m_WindowW = nWindowW;
    m_WindowH = nWindowH;

    if(m_WindowW && m_WindowH){
        // try to adjust the current window size
        SDL_DisplayMode stDesktop;
        if(!SDL_GetDesktopDisplayMode(0, &stDesktop)){
            m_WindowW = std::min(m_WindowW , stDesktop.w);
            m_WindowH = std::min(m_WindowH , stDesktop.h);
        }
    }

    m_Window = SDL_CreateWindow("MIR2X-V0.1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_WindowW, m_WindowH, nFlags);
    if(!m_Window){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Failed to create SDL window handler: %s", SDL_GetError());
    }

    m_Renderer = SDL_CreateRenderer(m_Window, -1, 0);

    if(!m_Renderer){
        SDL_DestroyWindow(m_Window);
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Failed to create SDL renderer: %s", SDL_GetError());
    }

    SetWindowIcon();
    PushColor(0, 0, 0, 0);
    PushBlendMode(SDL_BLENDMODE_NONE);

    SDL_StartTextInput();
}

void SDLDevice::DrawTextureEx(SDL_Texture *pTexture,
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

        SDL_RenderCopyEx(m_Renderer, pTexture, &stSrc, &stDst, fAngle, &stCenter, SDL_FLIP_NONE);
    }
}

TTF_Font *SDLDevice::DefaultTTF(uint8_t /* nFontSize */)
{
    static std::vector<uint32_t> s_DefaultTTFData
    {
        // binary format for .inc file
        // there could be serveral zeros at end
        // [0] : dataLen + N in bytes
        // [x] : data
        // [N] : zeros
        #include "defaultttf.inc"
    };
    return nullptr;
}
