/*
 * =====================================================================================
 *
 *       Filename: game.cpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 06/21/2017 23:31:26
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

#include <thread>
#include <future>

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
    // fullfil the time cq
    for(size_t nIndex = 0; nIndex < m_DelayTimeCQ.Capacity(); ++nIndex){
        m_DelayTimeCQ.PushHead(1000.0 / m_FPS);
    }

    // load PNGTexDB
    extern PNGTexDBN    *g_PNGTexDBN;
    extern PNGTexOffDBN *g_HeroGfxDBN;
    extern PNGTexOffDBN *g_WeaponDBN;
    extern PNGTexOffDBN *g_PNGTexOffDBN;
    extern FontexDBN    *g_FontexDBN;
    extern XMLConf      *g_XMLConf;
    extern Log          *g_Log;

    // texture path
    auto pNode = g_XMLConf->GetXMLNode("Root/Texture/PNGTexDBN");
    if(!pNode){
        g_Log->AddLog(LOGTYPE_WARNING, "No PNGTexDBN path found in configuration.");
        throw std::error_code();
    }
    g_Log->AddLog(LOGTYPE_INFO, "PNGTexDBN path: %s", pNode->GetText());
    g_PNGTexDBN->Load(pNode->GetText());

    // fontex load
    pNode = g_XMLConf->GetXMLNode("Root/Font/FontexDBN");
    if(!pNode){
        g_Log->AddLog(LOGTYPE_WARNING, "No FontexDBN path found in configuration.");
        throw std::error_code();
    }
    g_Log->AddLog(LOGTYPE_INFO, "FontexDBN path: %s", pNode->GetText());
    g_FontexDBN->Load(pNode->GetText());

    // texture with offset load
    pNode = g_XMLConf->GetXMLNode("Root/Texture/PNGTexOffDBN");
    if(!pNode){
        g_Log->AddLog(LOGTYPE_WARNING, "No PNGTexOffDBN path found in configuration.");
        throw std::error_code();
    }
    g_Log->AddLog(LOGTYPE_INFO, "PNGTexOffDBN path: %s", pNode->GetText());
    g_PNGTexOffDBN->Load(pNode->GetText());

    // hero gfx resource
    pNode = g_XMLConf->GetXMLNode("Root/Texture/HeroGfxDBN");
    if(!pNode){
        g_Log->AddLog(LOGTYPE_WARNING, "No HeroGfxDBN path found in configuration.");
        throw std::error_code();
    }
    g_Log->AddLog(LOGTYPE_INFO, "HeroGfxDBN path: %s", pNode->GetText());
    g_HeroGfxDBN->Load(pNode->GetText());

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
{
}

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

    auto fLastMS = GetTimeTick();
    while(m_CurrentProcess->ID() != PROCESSID_EXIT){
        PollASIO();
        ProcessEvent();
        
        double fCurrentMS = GetTimeTick();
        // if(m_NetPackTick > 0.0){
        //     if(fCurrentMS - m_NetPackTick > 15.0 * 1000){
        //         std::exit(0);
        //     }
        // }

        Update(fCurrentMS - fLastMS);
        Draw();

        m_DelayTimeCQ.PushHead(fCurrentMS - fLastMS);
        fLastMS = fCurrentMS;

        // try to expect next delay time interval
        double fTimeSum = 0.0;
        for(m_DelayTimeCQ.Reset(); !m_DelayTimeCQ.Done(); m_DelayTimeCQ.Forward()){
            fTimeSum += m_DelayTimeCQ.Current();
        }

        double fExpectedTime = (1.0 + m_DelayTimeCQ.Size()) * 1000.0 / m_FPS - fTimeSum;
        EventDelay(fExpectedTime);
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
