/*
 * =====================================================================================
 *
 *       Filename: sdldevice.cpp
 *        Created: 03/07/2016 23:57:04
 *  Last Modified: 03/11/2016 23:11:46
 *
 *    Description: copy from flare-engine:
 *		   SDLDevice.h/cpp
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

#include "xmlext.hpp"
#include "sdldevice.hpp"

SDLDevice::SDLDevice(const XMLExt &stXMLExt)
    : m_Window(nullptr)
    , m_Renderer(nullptr)
{
    // TODO
    // won't support dynamically create/update context
    // context should be created at the very beginning
    Uint32 nFlags   = 0;
    int    nWindowW = 0;
    int    nWindowH = 0;

    try{
        if(stXMLExt.NodeAtob("Root/Configure/Window/FullScreen")){
            nFlags |= SDL_WINDOW_FULLSCREEN;
        }
    }catch(...){
        // it's ok to miss this index
    }

    try{
        nWindowW = stXMLExt.NodeAtoi("Root/Configure/Window/W");
        nWindowH = stXMLExt.NodeAtoi("Root/Configure/Window/H");
    }catch(...){
        // assign the proper size it later
    }

    if(!(nWindowW && nWindowH)){
        nWindowW = 800;
        nWindowH = 600;
    }

    if(nWindowW && nWindowH){
        // try to adjust the current window size
        SDL_DisplayMode stDesktop;
        if(!SDL_GetDesktopDisplayMode(0, &stDesktop)){
            nWindowW = std::min(nWindowW , stDesktop.w);
            nWindowH = std::min(nWindowH , stDesktop.h);
        }
    }

    m_Window = SDL_CreateWindow("MIR2X-V0.1",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, nWindowW, nWindowH, nFlags);

    if(!m_Window){
        throw std::system_error("Failed to crate window");
    }

    m_Renderer = SDL_CreateRenderer(window, -1, 0);
    if(!m_Renderer){
        SDL_DestroyWindow(m_Window);
        throw std::system_error("Failed to crate renderer");
    }

    SetWindowIcon();
}

SDLDevice::~SDLDevice()
{
    if(m_Window){
        SDL_DestroyWindow(m_Window);
    }

    if(m_Renderer){
        SDL_DestroyRenderer(m_Renderer);
    }

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

    SDL_Surface *pstSurface = SDL_CreateRGBSurfaceFrom(pRawData, 16, 16, 16, 16*2, 0x0f00, 0x00f0, 0x000f, 0xf000);

    // The icon is attached to the window pointer
    if(pstSurface){
        SDL_SetWindowIcon(window, pstSurface);
        SDL_FreeSurface(pstSurface);
    }
}

SDL_Texture *SDLDevice::CreateTexture(const void *pMem, int nSize)
{
    // TODO
    //
    // currently it doesn't support dynamic set of context
    // because all textures are based on current m_Renderer
    //
    // if it's changed
    // all the texture need to be re-load
    //
    if(pMem == nullptr || nSize <= 0){ return nullptr; }

    SDL_RWops   *pstRWops   = nullptr;
    SDL_Surface *pstSurface = nullptr;
    SDL_Texture *pstTexture = nullptr;

    pstRWops = SDL_RWFromConstMem(pMem);
    if(pstRWops){
        pstSurface = SDL_LoadPNG_RW(pstRWops);
        if(pstSurface){
            pstTexture = SDL_CreateTextureFromSurface(m_Renderer, pstSurface);
        }
    }

    if(pstRWops){
        SDL_FreeRW(pstRWops);
    }

    if(pstSurface){
        SDL_FreeSurface(pstSurface);
    }

    return pstTexture;
}

void SDLDevice::DrawTexture(SDL_Texture *pstTexture, int nX, int nY)
{
    if(pstTexture){

        SDL_Rect stSrc;
        SDL_Rect stDst;

        int nW, nH;

        if(!SDL_QueryTexture(pstTexture, nullptr, nullptr, &nW, &nH)){

            // TODO
            // not sure whether need to adjust for boundary
            // hope not!
            stSrc = { 0,  0, nW, nH};
            stDst = {nX, nY, nW, nH};

            SDL_RenderCopy(m_Renderer, pstTexture, &stSrc, &stDst);
        }
    }
}

SDL_Texture *SDLDevice::CreateTexture(const uint8_t *pRawData, size_t nLen)
{
    if(pRawData && nLen > 0){
        auto pSurface = IMG_LoadPNG_RW(SDL_RWFromConstMem(pRawData, nLen));
        if(pSurface){
            auto pTexture = SDL_CreateTextureFromSurface(m_Renderer, pSurface);
            SDL_FreeSurface(pSurface);
            return pTexture;
        }
    }
    return nullptr;
}
