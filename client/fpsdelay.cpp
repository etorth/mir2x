#include "game.hpp"

Game::FPSDelay()
{
    double fNextUpdateTime = m_FPSCount * 1000.0 / m_FPS;
    double fCurrentTime    = 1.0 * SDL_GetTicks();
    Uint32 nHalfWaitTime   = (Uint32)((fNextUpdateTime - fCurrentTime) / 2.0 );
    
    if(fCurrentTime < fNextUpdateTime && nWaitTime != 0){
        SDL_Delay(nWaitTime);
    }
}
