/*
 * =====================================================================================
 *
 *       Filename: game.cpp
 *        Created: 8/12/2015 9:59:15 PM
 *  Last Modified: 01/25/2016 19:07:52
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

#include "game.hpp"

Game::Game()
    , m_CurrentProcessID(PROCESSID_NULL)
    , m_Window(nullptr)
    , m_Renderer(nullptr)
{
}

Game::~Game()
{
    SDL_DestroyRenderer(m_Renderer);
    SDL_DestroyWindow(m_Window);
}

void Game::Init()
{
    m_WindowFlag = 0
        | ((int)(m_Config->GetBool("Root/Window/FullScreen")) * SDL_WINDOW_FULLSCREEN)
        | ((int)(m_Config->GetBool("Root/Window/UseOpenGL" )) * SDL_WINDOW_OPENGL)
        ;

    m_Window = SDL_CreateWindow(m_Config()->GetString("Root/Window/Caption"),
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,    
            m_Config()->GetInt("Root/Window/SizeW"),
            m_Config()->GetInt("Root/Window/SizeH"),
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
    SwitchProcess(PROCESSID_LOGO);
}

void Game::MainLoop()
{
    while(m_CurrentProcessID != PROCESSID_EXIT){
        while(true){
            SDL_Event stEvent;
            if(SDL_PollEvent(&stEvent)){
                ProcessEvent(&stEvent);
            }else{
                if(!FPSDelay()){
                    break;
                }
            }
        }
        Update();
        Draw();
    }
}
