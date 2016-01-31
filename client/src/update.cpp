#include "game.hpp"

void Game::Update()
{
    switch(m_CurrentProcessID){
        case PROCESSID_LOGO:  UpdateOnLogo(SDL_GetTicks());  DrawOnLogo();  break;
        case PROCESSID_SYRC:  UpdateOnSyrc(SDL_GetTicks());  DrawOnSyrc();  break;
        case PROCESSID_LOGIN: UpdateOnLogin(SDL_GetTicks()); DrawOnLogin(); break;
        case PROCESSID_RUN:   UpdateOnRun(SDL_GetTicks());   DrawOnRun();   break;
        default: break;
    }
    m_FPSCount++;
}
