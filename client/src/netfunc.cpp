/*
 * =====================================================================================
 *
 *       Filename: netfunc.cpp
 *        Created: 01/15/2016 06:37:36
 *  Last Modified: 01/23/2016 02:10:01
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

#include "game.hpp"

void Game::NetFunc(void *pData)
{
    Game *pGame = (Game *)pData;
    if(pGame){
        pGame->RunASIO();
    }
}
