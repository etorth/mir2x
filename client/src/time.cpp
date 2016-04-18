/*
 * =====================================================================================
 *
 *       Filename: time.cpp
 *        Created: 03/19/2016 00:35:32
 *  Last Modified: 04/18/2016 00:02:47
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

double Game::GetTimeMS()
{
    return SDL_GetPerformanceCounter() * 1000.0 / SDL_GetPerformanceFrequency();
}

void Game::EventDelay(double fDelayMS)
{
    double fStartDelayMS = GetTimeMS();

    while(true){

        // always try to poll it
        PollASIO();

        // everytime firstly try to process all pending events
        ProcessEvent();

        double fCurrentMS = GetTimeMS();

        if(fCurrentMS - fStartDelayMS > fDelayMS){
            break;
        }

        // still we need to delay, so just delay
        // delay half the time
        //
        // TODO
        //
        // here we check the delay time
        // since SDL_Delay(0) may run into problem
        //
        Uint32 nDelayMSCount = (Uint32)(fCurrentMS - fStartDelayMS / 2.0);

        if(nDelayMSCount > 0){
            SDL_Delay(nDelayMSCount);
        }
    }
}
