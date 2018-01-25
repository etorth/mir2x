/*
 * =====================================================================================
 *
 *       Filename: game.cpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 01/25/2018 11:41:24
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
#include "initview.hpp"
#include "sysconst.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "fontexdbn.hpp"
#include "processrun.hpp"
#include "processlogo.hpp"
#include "processsyrc.hpp"
#include "processlogin.hpp"
#include "pngtexoffdbn.hpp"
#include "servermessage.hpp"

Game::Game()
    : m_ServerDelay( 0.00)
    , m_NetPackTick(-1.00)
    , m_RequestProcess(PROCESSID_NONE)
    , m_CurrentProcess(nullptr)
{
    InitView(10);
    extern SDLDevice *g_SDLDevice;
    g_SDLDevice->CreateMainWindow();
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

void Game::MainLoop()
{
    SwitchProcess(PROCESSID_LOGO);
    InitASIO();

    auto fDelayDraw   = (1000.0 / (1.0 * SYS_DEFFPS)) / 6.0;
    auto fDelayUpdate = (1000.0 / (1.0 * SYS_DEFFPS)) / 7.0;
    auto fDelayLoop   = (1000.0 / (1.0 * SYS_DEFFPS)) / 8.0;

    auto fLastDraw   = SDL_GetTicks() * 1.0;
    auto fLastUpdate = SDL_GetTicks() * 1.0;
    auto fLastLoop   = SDL_GetTicks() * 1.0;

    while(true){
        SwitchProcess();
        if(true
                && m_CurrentProcess
                && m_CurrentProcess->ID() != PROCESSID_EXIT){

            if(m_NetPackTick > 0.0){
                if(SDL_GetTicks() * 1.0 - m_NetPackTick > 15.0 * 1000){
                    // std::exit(0);
                }
            }

            auto fCurrUpdate = 1.0 * SDL_GetTicks();
            if(fCurrUpdate - fLastUpdate > fDelayUpdate){
                auto fPastUpdate = fCurrUpdate - fLastUpdate;
                fLastUpdate = fCurrUpdate;
                Update(fPastUpdate);
            }

            auto fCurrDraw = 1.0 * SDL_GetTicks();
            if(fCurrDraw - fLastDraw > fDelayDraw){
                fLastDraw = fCurrDraw;
                Draw();
            }

            auto fCurrLoop = 1.0 * SDL_GetTicks();
            auto fPastLoop = fCurrLoop - fLastLoop;

            fLastLoop = fCurrLoop;
            EventDelay(fDelayLoop - fPastLoop);
        }else{
            // need to exit
            // jump to the invalid process
            break;
        }
    }
}

void Game::EventDelay(double fDelayMS)
{
    double fStartDelayMS = SDL_GetTicks() * 1.0;
    while(true){

        // always try to poll it
        PollASIO();

        // everytime firstly try to process all pending events
        ProcessEvent();

        double fCurrentMS = SDL_GetTicks() * 1.0;
        double fDelayDone = fCurrentMS - fStartDelayMS;

        if(fDelayDone >= fDelayMS){ break; }

        // here we check the delay time
        // since SDL_Delay(0) may run into problem

        auto nDelayMSCount = (Uint32)(std::lround((fDelayMS - fDelayDone) * 0.50));
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

void Game::OnServerMessage(uint8_t nHC, const uint8_t *pData, size_t nDataLen)
{
    // 1. update the time when last message received
    m_NetPackTick = SDL_GetTicks() * 1.0;

    // 2. handle messages
    switch(nHC){
        case SM_UPDATEHP:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_UPDATEHP(pData, nDataLen);
                }
                break;
            }
        case SM_DEADFADEOUT:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_DEADFADEOUT(pData, nDataLen);
                }
                break;
            }
        case SM_EXP:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_EXP(pData, nDataLen);
                }
                break;
            }
        case SM_GOLD:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_GOLD(pData, nDataLen);
                }
                break;
            }
        case SM_FIREMAGIC:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_FIREMAGIC(pData, nDataLen);
                }
                break;
            }
        case SM_SHOWDROPITEM:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_SHOWDROPITEM(pData, nDataLen);
                }
                break;
            }
        case SM_PICKUPOK:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_PICKUPOK(pData, nDataLen);
                }
                break;
            }
        case SM_PING:
            {
                break;
            }
        case SM_LOGINOK:
            {
                SwitchProcess(m_CurrentProcess->ID(), PROCESSID_RUN);
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_LOGINOK(pData, nDataLen);
                }else{
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_INFO, "failed to jump into main loop");
                }
                break;
            }
        case SM_CORECORD:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_CORECORD(pData, nDataLen);
                }
                break;
            }
        case SM_LOGINFAIL:
            {
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "Login failed: ID = %d", (int)(((SMLoginFail *)(pData))->FailID));
                break;
            }
        case SM_ACTION:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_ACTION(pData, nDataLen);
                }
                break;
            }
        case SM_OFFLINE:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_OFFLINE(pData, nDataLen);
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

void Game::SwitchProcess()
{
    if(true
            && m_RequestProcess > PROCESSID_NONE
            && m_RequestProcess > PROCESSID_MAX){
        SwitchProcess((m_CurrentProcess ? m_CurrentProcess->ID() : PROCESSID_NONE), m_RequestProcess);
    }
    m_RequestProcess = PROCESSID_NONE;
}

void Game::SwitchProcess(int nNewID)
{
    SwitchProcess((m_CurrentProcess ? m_CurrentProcess->ID() : PROCESSID_NONE), nNewID);
}

void Game::SwitchProcess(int nOldID, int nNewID)
{
    delete m_CurrentProcess;
    m_CurrentProcess = nullptr;

    switch(nOldID)
    {
        case PROCESSID_NONE:
            {
                switch(nNewID)
                {
                    case PROCESSID_LOGO:
                        {
                            // on initialization
                            m_CurrentProcess = new ProcessLogo();
                            SDL_ShowCursor(0);
                            break;
                        }
                    case PROCESSID_EXIT:
                        {
                            // exit immediately when logo shows
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        case PROCESSID_LOGO:
            {
                switch(nNewID)
                {
                    case PROCESSID_SYRC:
                        {
                            // on initialization
                            m_CurrentProcess = new ProcessSyrc();
                            SDL_ShowCursor(1);
                            break;
                        }
                    case PROCESSID_EXIT:
                        {
                            // exit immediately when logo shows
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        case PROCESSID_SYRC:
            {
                switch(nNewID)
                {
                    case PROCESSID_LOGIN:
                        {
                            m_CurrentProcess = new ProcessLogin();
                            SDL_ShowCursor(1);
                            break;
                        }
                    default:
                        {
                            break;
                        }

                }
                break;
            }
        case PROCESSID_LOGIN:
            {
                switch(nNewID){
                    case PROCESSID_RUN:
                        {
                            m_CurrentProcess = new ProcessRun();
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
            }
        default:
            {
                break;
            }
    }
}
