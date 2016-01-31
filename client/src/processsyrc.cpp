/*
 * =====================================================================================
 *
 *       Filename: processsyrc.cpp
 *        Created: 8/14/2015 2:47:49 PM
 *  Last Modified: 01/23/2016 04:51:29
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



ProcessSyrc::ProcessSyrc(Game *pGame)
	: Process(Process::PROCESSID_SYRC, pGame)
    , m_TextureBackground(nullptr)
    , m_TextureProgressBar(nullptr)
    , m_Ratio(0)
    , m_Info({0, 14, 0}, {255, 255, 255, 0}, "Connecting...")
    // , m_TokenBoardConnect(800, true)
    // , m_TokenBoardConnectSucceed(800, true)
    // , m_CurrentTokenBoard(nullptr)
{
	m_Info.SetX((800 - m_Info.W()) / 2);
	m_Info.SetY(505);

    m_TextureBackground  = GetTextureManager()->RetrieveTexture(63, 0, 1);
    m_TextureProgressBar = GetTextureManager()->RetrieveTexture(63, 0, 2);
}

ProcessSyrc::~ProcessSyrc()
{}

void UpdateOnSyrc()
{
    // nothing to do
}

void Game::ProcessEventOnSyrc(SDL_Event *pEvent)
{
    if(pEvent){
        switch(pEvent->type){
            case SDL_KEYDOWN:
                {
                    if(pEvent->key.keysym.sym == SDLK_ESCAPE){
                        SwitchProcess(PROCESSID_LOGIN);
                    }
                    break;
                }
            case SDL_USEREVENT:
                {
                    ProcessUserEvent(pEvent);
                    break;
                }
            default:
                break;
        }
    }
}

void Game::DrawOnSyrc()
{
    SDL_Rect stRectSrc, stRectDst;
	int nW, nH;
	SDL_QueryTexture(GetGUITextureManager()->Retrieve(), nullptr, nullptr, &nW, &nH);

    stRectSrc.x = 0;
    stRectSrc.y = 0;
    stRectSrc.w = std::lround(nW * (GetFinishedOnSyrc() / 100.0));
	stRectSrc.h = nH;
    stRectDst.x = 112;
    stRectDst.y = 528;
    stRectDst.w = stRectSrc.w;
    stRectDst.h = stRectSrc.h;

    SDL_RenderCopy(m_Renderer, m_TextureProgressBar, &stRectSrc, &stRectDst);
    SDL_RenderCopy(m_Renderer, m_TextureBackground, nullptr, nullptr);

	m_Info.Draw();
}
