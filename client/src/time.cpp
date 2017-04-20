/*
 * =====================================================================================
 *
 *       Filename: time.cpp
 *        Created: 03/19/2016 00:35:32
 *  Last Modified: 04/19/2017 23:06:27
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

void Game::EventDelay(double fDelayMS)
{
    double fStartDelayMS = GetTimeTick();
    while(true){

        // always try to poll it
        PollASIO();

        // everytime firstly try to process all pending events
        ProcessEvent();

        double fCurrentMS = GetTimeTick();
        double fDelayDone = fCurrentMS - fStartDelayMS;

        if(fDelayDone > fDelayMS){ break; }

        // here we check the delay time
        // since SDL_Delay(0) may run into problem

        Uint32 nDelayMSCount = (Uint32)(std::lround((fDelayMS - fDelayDone) * 0.50));
        if(nDelayMSCount > 0){ SDL_Delay(nDelayMSCount); }
    }
}
