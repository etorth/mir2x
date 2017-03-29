/*
 * =====================================================================================
 *
 *       Filename: creaturenet.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 03/28/2017 16:15:03
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

#include <string>
#include <algorithm>
#include <tinyxml2.h>
#include <SDL2/SDL.h>

#include "creature.hpp"
#include "sysconst.hpp"

void Creature::OnActionState(int nAction, int nDirection, int nSpeed, int nX, int nY)
{
    switch(nAction){
        case ACTION_WALK:
            {

            }
        default:
            {
                break;
            }
    }
}
