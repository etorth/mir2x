/*
 * =====================================================================================
 *
 *       Filename: game.cpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 07/18/2017 15:15:41
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

#include <future>
#include <thread>

#include "log.hpp"
#include "game.hpp"
#include "xmlconf.hpp"
#include "pngtexdbn.hpp"
#include "fontexdbn.hpp"
#include "pngtexoffdbn.hpp"

Game::Game()
    : m_FPS(30.0)
    , m_ServerDelay( 0.00)
    , m_NetPackTick(-1.00)
    , m_CurrentProcess(nullptr)
{
    // load PNGTexDB
    extern PNGTexDBN    *g_ProgUseDBN;
    extern PNGTexDBN    *g_MapDBN;
    extern PNGTexOffDBN *g_HeroDBN;
    extern PNGTexOffDBN *g_MonsterDBN;
    extern PNGTexOffDBN *g_WeaponDBN;
    extern FontexDBN    *g_FontexDBN;
    extern XMLConf      *g_XMLConf;
    extern Log          *g_Log;

    // texture path
    auto pNode = g_XMLConf->GetXMLNode("Root/Texture/ProgUseDBN");
    if(!pNode){
        g_Log->AddLog(LOGTYPE_WARNING, "No ProgUseDBN path found in configuration.");
        throw std::error_code();
    }
    g_Log->AddLog(LOGTYPE_INFO, "ProgUseDBN path: %s", pNode->GetText());
    g_ProgUseDBN->Load(pNode->GetText());

    // map texutre path
    pNode = g_XMLConf->GetXMLNode("Root/Texture/MapDBN");
    if(!pNode){
        g_Log->AddLog(LOGTYPE_WARNING, "No MapDBN path found in configuration.");
        throw std::error_code();
    }
    g_Log->AddLog(LOGTYPE_INFO, "MapDBN path: %s", pNode->GetText());
    g_MapDBN->Load(pNode->GetText());

    // fontex load
    pNode = g_XMLConf->GetXMLNode("Root/Font/FontexDBN");
    if(!pNode){
        g_Log->AddLog(LOGTYPE_WARNING, "No FontexDBN path found in configuration.");
        throw std::error_code();
    }
    g_Log->AddLog(LOGTYPE_INFO, "FontexDBN path: %s", pNode->GetText());
    g_FontexDBN->Load(pNode->GetText());

    // hero gfx resource
    pNode = g_XMLConf->GetXMLNode("Root/Texture/HeroDBN");
    if(!pNode){
        g_Log->AddLog(LOGTYPE_WARNING, "No HeroDBN path found in configuration.");
        throw std::error_code();
    }
    g_Log->AddLog(LOGTYPE_INFO, "HeroDBN path: %s", pNode->GetText());
    g_HeroDBN->Load(pNode->GetText());

    // monster resource
    pNode = g_XMLConf->GetXMLNode("Root/Texture/MonsterDBN");
    if(!pNode){
        g_Log->AddLog(LOGTYPE_WARNING, "No MonsterDBN path found in configuration.");
        throw std::error_code();
    }
    g_Log->AddLog(LOGTYPE_INFO, "MonsterDBN path: %s", pNode->GetText());
    g_MonsterDBN->Load(pNode->GetText());

    // weapon gfx resource
    pNode = g_XMLConf->GetXMLNode("Root/Texture/WeaponDBN");
    if(!pNode){
        g_Log->AddLog(LOGTYPE_WARNING, "No WeaponDBN path found in configuration.");
        throw std::error_code();
    }
    g_Log->AddLog(LOGTYPE_INFO, "WeaponDBN path: %s", pNode->GetText());
    g_WeaponDBN->Load(pNode->GetText());
}

Game::~Game()
{}

void Game::ProcessEvent()
{
    if(m_CurrentProcess){
        SDL_Event stEvent;
        while(SDL_PollEvent(&stEvent)){
            m_CurrentProcess->ProcessEvent(stEvent);
        }
    }
}

void Game::Update(double fDeltaMS)
{
    if(m_CurrentProcess){
        m_CurrentProcess->Update(fDeltaMS);
    }
}

void Game::Draw()
{
    if(m_CurrentProcess){
        m_CurrentProcess->Draw();
    }
}

void Game::MainLoop()
{
    SwitchProcess(PROCESSID_LOGO);
    InitASIO();

    auto fDelayDraw   = 1000.0 / m_FPS / 5.0;
    auto fDelayUpdate = 1000.0 / m_FPS / 6.0;
    auto fDelayLoop   = 1000.0 / m_FPS / 7.0;

    auto fLastDraw   = GetTimeTick();
    auto fLastUpdate = GetTimeTick();

    while(m_CurrentProcess->ID() != PROCESSID_EXIT){

        if(m_NetPackTick > 0.0){
            if(GetTimeTick() - m_NetPackTick > 15.0 * 1000){
                // std::exit(0);
            }
        }

        // check if need to update logic
        {
            auto fPastUpdate = GetTimeTick() - fLastUpdate;
            if(fPastUpdate >= fDelayUpdate){
                fLastUpdate = GetTimeTick();
                Update(fPastUpdate);
            }
        }

        // check if need to update screen
        {
            auto fPastDraw = GetTimeTick() - fLastDraw;
            if(fPastDraw >= fDelayDraw){
                fLastDraw = GetTimeTick();
                Draw();
            }
        }

        EventDelay(fDelayLoop);
    }
}

void Game::EventDelay(double fDelayMS)
{
    double fStartDelayMS = GetTimeTick();
    while(true){

        // always try to poll it
        PollASIO();

        // everytime firstly try to process all pending events
        ProcessEvent();

        double fCurrentMS = GetTimeTick();
        double fDelayDone = fCurrentMS - fStartDelayMS;

        if(fDelayDone > fDelayMS){ break; }

        // here we check the delay time
        // since SDL_Delay(0) may run into problem

        Uint32 nDelayMSCount = (Uint32)(std::lround((fDelayMS - fDelayDone) * 0.50));
        if(nDelayMSCount > 0){ SDL_Delay(nDelayMSCount); }
    }
}

void Game::InitASIO()
{
    // this function will run in another thread
    // make sure there is no data race

    // TODO
    // may need lock here since g_XMLConf may used in main thread also
    std::string szIP;
    std::string szPort;

    extern XMLConf *g_XMLConf;
    auto p1 = g_XMLConf->GetXMLNode("/Root/Network/Server/IP"  );
    auto p2 = g_XMLConf->GetXMLNode("/Root/Network/Server/Port");

    if(p1 && p2 && p1->GetText() && p2->GetText()){
        szIP   = p1->GetText();
        szPort = p2->GetText();
    }else{
        szIP   = "127.0.0.1";
        szPort = "5000";
    }

    m_NetIO.InitIO(szIP.c_str(), szPort.c_str(),
        [this](uint8_t nHC, const uint8_t *pData, size_t nDataLen){
            // core should handle on fully recieved message from the serer
            // previously there are two steps (HC, Body) seperately handled, error-prone
            OnServerMessage(nHC, pData, nDataLen);
        }
    );
}

void Game::PollASIO()
{
    m_NetIO.PollIO();
}

void Game::StopASIO()
{
    m_NetIO.StopIO();
}
