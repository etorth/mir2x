/*
 * =====================================================================================
 *
 *       Filename: game.cpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 03/10/2016 18:53:33
 *
 *    Description: public API for class game only
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
    : m_XMLExt()
    , m_SDLDevice(nullptr)
    , m_CurrentProcessID(PROCESSID_NULL)
    , m_Window(nullptr)
    , m_Renderer(nullptr)
{
    std::srand((unsigned int)std::time(nullptr));

    // if failed, it will throw directly
    m_XMLExt = new XMLExt();

    if(false
            || m_XMLExt->Load("./configuration.xml")
            || m_XMLExt->Load("./configure.xml"    )
            || m_XMLExt->Load("./config.xml"       )
            || m_XMLExt->Load("./conf.xml"         )){
        throw std::invalid_argument("No configuration found");
    }

    try{
        m_SDLDevice = new SDLDevice(*m_XMLExt);
        m_NetIO     = new NetIO(stXMLExt, [this](){ ReadHC(); })
    }catch(...){
        delete m_XMLExt;
        delete m_SDLDevice;
        throw;
    }
}

Game::~Game()
{
    delete m_XMLExt;
    delete m_SDLDevice;
}

void Game::MainLoop()
{
    // for first time entry, setup threads etc..
    //
    SwitchProcess(PROCESSID_LOGO);

    std::asyc([this](){ RunASIO(); });

    while(m_CurrentProcessID != PROCESSID_EXIT){
        while(true){
            SDL_Event stEvent;
            if(SDL_PollEvent(&stEvent)){
                ProcessEvent(&stEvent);
            }else{
                if(!FPSDelay()){
                    break;
                }
            }
        }
        Update();
        Draw();
    }
}
