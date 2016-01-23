#include "game.hpp"

void Game::ProcessEventOnLogo(SDL_Event *pEvent)
{
    if(true
            && pEvent
            && pEvent->type == SDL_KEYDOWN
            && pEvent->key.keysym.sym == SDLK_ESCAPE
      ){
        SwitchProcess(PROCESSID_LOGO, PROCESSID_SYRC);
    }
}

void Game::ProcessEventOnSyrc(SDL_Event *pEvent)
{
    // do nothing, just wait
}

void Game::ProcessEventOnLogin(SDL_Event *pEvent)
{
    if(pEvent){
        m_IDBox.ProcessEvent(pEvent);
        m_PasswordBox.ProcessEvent(pEvent);

        if(false
                || m_Button1.ProcessEvent(pEvent)
                || m_Button2.ProcessEvent(pEvent)
                || m_Button3.ProcessEvent(pEvent)
                || m_Button4.ProcessEvent(pEvent)
          ){
            return;
        }
    }
}

void Game::ProcessEventOnRun(SDL_Event *pEvent)
{
}
