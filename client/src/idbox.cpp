/*
 * =====================================================================================
 *
 *       Filename: idbox.cpp
 *        Created: 07/16/2017 19:06:25
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

#include "idbox.hpp"
bool IDBox::ProcessEvent(const SDL_Event &rstEvent, bool *pValid)
{
    if(pValid && !(*pValid)){ return false; }

    // even not focused
    // should also accept some events
    // like the insert sign when pointer inside idbox

    switch(rstEvent.type){
        case SDL_KEYDOWN:
            {
                switch(rstEvent.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            if(Focus()){
                                if(m_OnEnter){  m_OnEnter();    }
                                if(pValid   ){ *pValid = false; }
                                return true;
                            }
                            break;
                        }
                    case SDLK_TAB:
                        {
                            if(Focus()){
                                if(m_OnTab){  m_OnTab();      }
                                if(pValid ){ *pValid = false; }
                                return true;
                            }
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
            }
    }
    return InputBoard::ProcessEvent(rstEvent, pValid);
}
