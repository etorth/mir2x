/*
 * =====================================================================================
 *
 *       Filename: game.cpp
 *        Created: 8/12/2015 9:59:15 PM
 *  Last Modified: 01/23/2016 05:18:46
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

Game::Game()
    : m_CurrentProcessID(PROCESSID_NULL)
{}

Game::~Game()
{}

void Game::Init()
{
    SwitchProcess(PROCESSID_LOGO);
}

void Game::MainLoop()
{
    while(m_CurrentProcessID != PROCESSID_EXIT){
        SDL_Event stEvent;
        if(SDL_WaitEvent(&stEvent)){
            ProcessEvent(&stEvent);
        }
    }
}
