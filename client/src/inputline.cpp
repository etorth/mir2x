/*
 * =====================================================================================
 *
 *       Filename: inputline.cpp
 *        Created: 06/19/2017 11:29:06
 *  Last Modified: 06/19/2017 11:38:07
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
    if(true
            && Focus()
            && rstEvent.type == SDL_KEYDOWN
            && rstEvent.key.keysym.sym == SDLK_RETURN){

        return true;
    }

    return InputBoard::ProcessEvent(rstEvent, pValid);
}
