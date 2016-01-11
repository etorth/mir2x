/*
 * =====================================================================================
 *
 *       Filename: devicemanager.cpp
 *        Created: 6/17/2015 6:05:01 PM
 *  Last Modified: 08/21/2015 3:02:28 AM
 *
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
#include "configurationmanager.hpp"
#include "devicemanager.hpp"
#include <cstdlib>
#include <ctime>
#include <SDL.h>

bool DeviceManager::Init()
{
    std::srand((unsigned int)std::time(nullptr));
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
        SDL_Log("Could not initialize SDL: %s", SDL_GetError());
        SDL_Quit();
        exit(0);
    }

    m_WindowFlag = 0
        | ((int)(GetConfigurationManager()->GetBool("Root/Window/FullScreen")) * SDL_WINDOW_FULLSCREEN)
        | ((int)(GetConfigurationManager()->GetBool("Root/Window/UseOpenGL" )) * SDL_WINDOW_OPENGL)
        ;

    m_Window = SDL_CreateWindow(GetConfigurationManager()->GetString("Root/Window/Caption"),
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,    
            GetConfigurationManager()->GetInt("Root/Window/SizeW"),
            GetConfigurationManager()->GetInt("Root/Window/SizeH"),
            m_WindowFlag | SDL_WINDOW_SHOWN);
    if (m_Window == nullptr){
        SDL_Log("Could not create window: %s", SDL_GetError());
        SDL_Quit();
        exit(0);
    }

    m_RendererFlag = 0
        | SDL_RENDERER_ACCELERATED
        | SDL_RENDERER_PRESENTVSYNC
        | SDL_RENDERER_TARGETTEXTURE
        ;
    m_Renderer = SDL_CreateRenderer(m_Window, -1, m_RendererFlag);

    if (m_Renderer == nullptr){
        SDL_Log("Could not create renderer: %s", SDL_GetError());
        SDL_Quit();
        exit(0);
    }
    return true;
}

void DeviceManager::Release()
{
    SDL_DestroyRenderer(m_Renderer);
    SDL_DestroyWindow(m_Window);
}

SDL_Renderer *DeviceManager::GetRenderer()
{
    return m_Renderer;
}

DeviceManager *GetDeviceManager()
{
    static DeviceManager deviceManager;
    return &deviceManager;
}

void DeviceManager::SetRenderDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(m_Renderer, r, g, b, a);
}

void DeviceManager::Clear()
{
    SDL_RenderClear(m_Renderer);
}

void DeviceManager::Present()
{
    SDL_RenderPresent(m_Renderer);
}

int DeviceManager::WindowSizeW()
{
    int nW;
    SDL_GetWindowSize(m_Window, &nW, nullptr);
    return nW;
}

int DeviceManager::WindowSizeH()
{
    int nH;
    SDL_GetWindowSize(m_Window, &nH, nullptr);
    return nH;
}
