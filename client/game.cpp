/*
 * =====================================================================================
 *
 *       Filename: game.cpp
 *        Created: 8/12/2015 9:59:15 PM
 *  Last Modified: 01/11/2016 23:04:35
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
    : m_CurrentProcess(nullptr)
	, m_ProcessLogin(this)
	, m_ProcessLogo(this)
	, m_ProcessSyrc(this)
	, m_ProcessRun(this)
{}

Game::~Game()
{}

void Game::Init()
{
    m_CurrentProcess = &m_ProcessLogo;
	m_CurrentProcess->Enter();

}

void Game::MainLoop()
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

void Game::StartSystem()
{
    std::srand((unsigned int)std::time(nullptr));

    GetConfigurationManager()->Init();
    GetTextureManager()->Init();
    GetDeviceManager()->Init();
    GetMessageManager()->Init();

    GetFontTextureManager()->Init();
    GetEmoticonManager()->Init();
}

ProcessRun *Game::OnProcessRun()
{
    return &m_ProcessRun;
}
