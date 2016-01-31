/*
 * =====================================================================================
 *
 *       Filename: timerfunc.cpp
 *        Created: 01/15/2016 06:12:48
 *  Last Modified: 01/15/2016 06:37:05
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

Uint32 Game::TimerFunc(Uint32, void *)
{
    SDL_Event stEvent;
    stEvent.type = SDL_USEREVENT;
    stEvent.user.code = 0;
    if(SDL_PushEvent(&stEvent) != 1){
        SDL_Log("Could not push timer event: %s", SDL_GetError());
        SDL_Quit();
        exit(0);
    }
    return 1;
}
