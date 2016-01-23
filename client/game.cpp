/*
 * =====================================================================================
 *
 *       Filename: game.cpp
 *        Created: 8/12/2015 9:59:15 PM
 *  Last Modified: 01/16/2016 09:10:55
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
#include "devicemanager.hpp"
#include "texturemanager.hpp"
#include "fonttexturemanager.hpp"
#include "emoticonmanager.hpp"
#include "configurationmanager.hpp"
#include "messagemanager.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <cstdlib>
#include <ctime>

Game::Game()
    : m_CurrentProcessID(PROCESSID_NULL)
{}

Game::~Game()
{}

void Game::Init()
{
    m_StateStartTick   = SDL_GetTicks();
    m_CurrentProcessID = PROCESSID_LOGO;
}


void Game::EventDelay()
{
    Uint32 nEstimateTime = (Uint32)(m_InvokeCount * 1000.0 / m_FPS);
    Uint32 fNextUpdateTime = m_StartTime + nEstimateTime;

    while(true){
        SDL_Event stEvent;
        while(SDL_PollEvent(&stEvent)){
            ProcessEvent(&stEvent);
        }

        Uint32 nNowTime = SDL_GetTicks();
        if(nNextUpdateTime > nNowTime){
            if(SDL_WaitEventTimeout(&stEvent, (int)(nNextUpdateTime - nNowTime))){
                ProcessEvent(&stEvent);
            }
        }else{
            break;
        }
    }
}

void Game::MainLoop()
{
    while(m_CurrentProcessID != PROCESSID_NULL){
        m_InvokeCount++;
        EventDelay();
        Update();
        Draw();
    }
}

void Game::Update()
{
    switch(m_CurrentProcessID){
        case PROCESSID_LOGO:  UpdateOnLogo();  break;
        case PROCESSID_SYRC:  UpdateOnSyrc();  break;
        case PROCESSID_LOGIN: UpdateOnLogin(); break;
        case PROCESSID_RUN:   UpdateOnRun();   break;
        default: break;
    }
}

void Game::ProcessEvent(SDL_Event *stEvent)
{
    switch(m_CurrentProcessID){
        case PROCESSID_LOGO:  ProcessEventOnLogo();  break;
        case PROCESSID_SYRC:  ProcessEventOnSyrc();  break;
        case PROCESSID_LOGIN: ProcessEventOnLogin(); break;
        case PROCESSID_RUN:   ProcessEventOnRun();   break;
        default: break;
    }
}

void Game::Draw()
{
    switch(m_CurrentProcessID){
        case PROCESSID_LOGO:  DrawOnLogo();  break;
        case PROCESSID_SYRC:  DrawOnSyrc();  break;
        case PROCESSID_LOGIN: DrawOnLogin(); break;
        case PROCESSID_RUN:   DrawOnRun();   break;
        default: break;
    }
}


{
    while(!m_CurrentProcess->RequestQuit()){
        m_CurrentProcess->Update();
        if(m_CurrentProcess->RequestNewProcess()){
            m_CurrentProcess->ClearEvent();
            auto nOldID = m_CurrentProcess->ProcessID();
            auto nNewID = m_CurrentProcess->NextProcessID();
            SwitchProcess(nOldID, nNewID);
        }else{
            m_CurrentProcess->EventDelay();
        }
        GetDeviceManager()->Clear();
        m_CurrentProcess->Draw();
        GetDeviceManager()->Present();
    }
}

void Game::Clear()
{
    GetConfigurationManager()->Release();
    GetDeviceManager()->Release();
    GetMessageManager()->Release();
    GetTextureManager()->Release();
}

void Game::SwitchProcess(int nOldID, int nNewID)
{
    switch(nOldID){
        case Process::PROCESSID_LOGO:
            {
                if(nNewID == Process::PROCESSID_SYRC){
                    m_CurrentProcess->Exit();
                    m_CurrentProcess = &m_ProcessSyrc;
                    m_CurrentProcess->Enter();
                }
                break;
            }
        case Process::PROCESSID_SYRC:
            {
                if(nNewID == Process::PROCESSID_LOGO){
                    m_CurrentProcess->Exit();
                    m_CurrentProcess = &m_ProcessLogo;
                    m_CurrentProcess->Enter();
                }

                if(nNewID == Process::PROCESSID_LOGIN){
                    m_CurrentProcess->Exit();
                    m_CurrentProcess = &m_ProcessLogin;
                    m_CurrentProcess->Enter();
                }
                break;
            }
        case Process::PROCESSID_LOGIN:
            {
                if(nNewID == Process::PROCESSID_RUN){
                    m_CurrentProcess->Exit();
                    m_CurrentProcess = &m_ProcessRun;
                    m_CurrentProcess->Enter();
                }
                break;
            }
        default:
			break;
    }
}

void Game::ProcessEventOnLogo(SDL_Event *pEvent)
{
    if(true
            && pEvent
            && pEvent->type == SDL_KEYDOWN
            && pEvent->key.keysym.sym == SDLK_ESCAPE
      ){
        SwitchProcess(PROCESSID_LOGO, PROCESSID_SYRC);
    }
}

void Game::ProcessEventOnSyrc(SDL_Event *pEvent)
{
    // do nothing, just wait
}

void Game::ProcessEventOnLogin(SDL_Event *pEvent)
{
}
