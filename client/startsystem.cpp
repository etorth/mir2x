/*
 * =====================================================================================
 *
 *       Filename: startsystem.cpp
 *        Created: 01/15/2016 05:40:29
 *  Last Modified: 01/15/2016 06:36:24
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

void Game::StartSystem()
{
    std::srand((unsigned int)std::time(nullptr));

    Uint32 nSDLFlag = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENT ;
    if(SDL_Init(nSDLFlag) < 0){
        SDL_Log("Could not initialize SDL: %s", SDL_GetError());
        SDL_Quit();
        exit(0);
    }

    if(m_NetEvent == (Uint32)(-1)){
        SDL_Log("Could not register new event type: %s", SDL_GetError());
        SDL_Quit();
        exit(0);
    }

    if(!SDL_AddTimer(2, Game::TimerFunc, this)){
        SDL_Log("Could not add timer: %s", SDL_GetError());
        SDL_Quit();
        exit(0);
    }

    m_NetThread = SDL_CreateThread(Game::NetFunc, "NetThread", this);
    if(!m_NetThread){
        SDL_Log("Could not add net message event: %s", SDL_GetError());
        SDL_Quit();
        exit(0);
    }






    GetConfigurationManager()->Init();
    GetTextureManager()->Init();
    GetDeviceManager()->Init();
    GetMessageManager()->Init();

    GetFontTextureManager()->Init();
    GetEmoticonManager()->Init();
}

