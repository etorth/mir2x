/*
 * =====================================================================================
 *
 *       Filename: inputline.cpp
 *        Created: 06/19/2017 11:29:06
 *  Last Modified: 06/19/2017 23:18:57
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

#include "inputline.hpp"

bool InputLine::ProcessEvent(const SDL_Event &rstEvent, bool *pValid)
{
    if(pValid && !(*pValid)){ return false; }
    switch(rstEvent.type){
        case SDL_KEYDOWN:
            {
                switch(rstEvent.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(m_IME && m_IME->Focus()){
                                return true;
                            }

                            if(Focus() && m_TabFunc){
                                m_TabFunc();
                                return true;
                            }

                            // for other cases we won't capture the event
                            // and pass to InputBoard for handling
                            break;
                        }
                    case SDLK_RETURN:
                        {
                            if(m_IME && m_IME->Focus()){
                                return true;
                            }

                            if(Focus() && m_ReturnFunc){
                                m_ReturnFunc();
                                return true;
                            }

                            // for other cases we won't capture the event
                            // and pass to InputBoard for handling
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }

                break;
            }
        default:
            {
                break;
            }
    }

    // fallback to use InputBoard event handling
    return InputBoard::ProcessEvent(rstEvent, pValid);
}
