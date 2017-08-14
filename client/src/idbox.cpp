/*
 * =====================================================================================
 *
 *       Filename: idbox.cpp
 *        Created: 07/16/2017 19:06:25
 *  Last Modified: 08/14/2017 00:08:05
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
#include "idbox.hpp"
bool IDBox::ProcessEvent(const SDL_Event &rstEvent, bool *pValid)
{
    if(pValid && !(*pValid)){ return false; }
    if(!Focus()){ return false; }

    switch(rstEvent.type){
        case SDL_KEYDOWN:
            {
                switch(rstEvent.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            if(m_OnEnter){  m_OnEnter();    }
                            if(pValid   ){ *pValid = false; }
                            return true;
                        }
                    case SDLK_TAB:
                        {
                            if(m_OnTab){  m_OnTab();      }
                            if(pValid ){ *pValid = false; }
                            return true;
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
