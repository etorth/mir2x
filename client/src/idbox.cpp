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
bool IDBox::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    // even not focused
    // should also accept some events
    // like the insert sign when pointer inside idbox

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            if(Focus()){
                                if(m_OnEnter){
                                    m_OnEnter();
                                }
                                return true;
                            }
                            return false;
                        }
                    case SDLK_TAB:
                        {
                            if(Focus()){
                                if(m_OnTab){
                                    m_OnTab();
                                }
                                return true;
                            }
                            return false;
                        }
                    default:
                        {
                            return false;
                        }
                }
            }
        default:
            {
                return InputBoard::processEvent(event, valid);
            }
    }
}
