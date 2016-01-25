#include "game.hpp"

void Game::ProcessEvent(SDL_Event *pEvent)
{
    if(pEvent){
        if(pEvent->type == SDL_USEREVENT){
            // network or timer event
            if(pEvent->user.code < 0){
                switch(m_CurrentProcessID){
                    case PROCESSID_LOGO:  UpdateOnLogo();  DrawOnLogo();  break;
                    case PROCESSID_SYRC:  UpdateOnSyrc();  DrawOnSyrc();  break;
                    case PROCESSID_LOGIN: UpdateOnLogin(); DrawOnLogin(); break;
                    case PROCESSID_RUN:   UpdateOnRun();   DrawOnRun();   break;
                    default: break;
                }
            }else{
                ProcessNetEvent((uint32_t)(pEvent->user.code),
                        (uint32_t)(pEvent->user.data1), pEvent->user.data2);
            }
        }else{
            // local event
            switch(m_CurrentProcessID){
                case PROCESSID_LOGO:  ProcessEventOnLogo();  break;
                case PROCESSID_SYRC:  ProcessEventOnSyrc();  break;
                case PROCESSID_LOGIN: ProcessEventOnLogin(); break;
                case PROCESSID_RUN:   ProcessEventOnRun();   break;
                default: break;
            }
        }
    }
}
