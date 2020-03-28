/*
 * =====================================================================================
 *
 *       Filename: client.cpp
 *        Created: 08/12/2015 09:59:15
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
#include <cinttypes>

#include "log.hpp"
#include "client.hpp"
#include "xmlconf.hpp"
#include "initview.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "fontexdb.hpp"
#include "sdldevice.hpp"
#include "debugboard.hpp"
#include "processnew.hpp"
#include "processrun.hpp"
#include "processlogo.hpp"
#include "processsync.hpp"
#include "pngtexoffdb.hpp"
#include "processlogin.hpp"
#include "servermessage.hpp"
#include "clientargparser.hpp"

extern Log *g_Log;
extern XMLConf *g_XMLConf;
extern SDLDevice *g_SDLDevice;
extern DebugBoard *g_DebugBoard;
extern ClientArgParser *g_ClientArgParser;

Client::Client()
    : m_ClientMonitor()
    , m_ServerDelay( 0.00)
    , m_NetPackTick(-1.00)
    , m_RequestProcess(PROCESSID_NONE)
    , m_CurrentProcess(nullptr)
{
    InitView(10);
    g_SDLDevice->CreateMainWindow();
}

Client::~Client()
{}

void Client::processEvent()
{
    if(m_CurrentProcess){
        SDL_Event stEvent;
        while(SDL_PollEvent(&stEvent)){
            m_CurrentProcess->processEvent(stEvent);
        }
    }
}

void Client::MainLoop()
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
        if(g_ClientArgParser->EnableClientMonitor){
            PrintMonitor();
        }

        SwitchProcess();
        if(true
                && m_CurrentProcess
                && m_CurrentProcess->ID() != PROCESSID_EXIT){

            if(m_NetPackTick > 0.0){
                if(SDL_GetTicks() * 1.0 - m_NetPackTick > 15.0 * 1000){
                    // std::exit(0);
                }
            }

            if(auto fCurrUpdate = 1.0 * SDL_GetTicks(); fCurrUpdate - fLastUpdate > fDelayUpdate){
                auto fPastUpdate = fCurrUpdate - fLastUpdate;
                fLastUpdate = fCurrUpdate;
                Update(fPastUpdate);
            }

            if(auto fCurrDraw = 1.0 * SDL_GetTicks(); fCurrDraw - fLastDraw > fDelayDraw){
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

void Client::EventDelay(double fDelayMS)
{
    double fStartDelayMS = SDL_GetTicks() * 1.0;
    while(true){

        // always try to poll it
        PollASIO();

        // everytime firstly try to process all pending events
        processEvent();

        double fCurrentMS = SDL_GetTicks() * 1.0;
        double fDelayDone = fCurrentMS - fStartDelayMS;

        if(fDelayDone >= fDelayMS){ break; }

        // here we check the delay time
        // since SDL_Delay(0) may run into problem

        auto nDelayMSCount = (Uint32)(std::lround((fDelayMS - fDelayDone) * 0.50));
        if(nDelayMSCount > 0){ SDL_Delay(nDelayMSCount); }
    }
}

void Client::InitASIO()
{
    // this function will run in another thread
    // make sure there is no data race

    // TODO
    // may need lock here since g_XMLConf may used in main thread also
    std::string szIP;
    std::string szPort;

    auto p1 = g_XMLConf->GetXMLNode("/Root/Network/Server/IP"  );
    auto p2 = g_XMLConf->GetXMLNode("/Root/Network/Server/Port");

    if(p1 && p2 && p1->GetText() && p2->GetText()){
        szIP   = p1->GetText();
        szPort = p2->GetText();
    }else{
        szIP   = "127.0.0.1";
        szPort = "5000";
    }

    m_NetIO.InitIO(szIP.c_str(), szPort.c_str(), [this](uint8_t nHC, const uint8_t *pData, size_t nDataLen)
    {
        // core should handle on fully recieved message from the serer
        // previously there are two steps (HC, Body) seperately handled, error-prone
        OnServerMessage(nHC, pData, nDataLen);
    });
}

void Client::PollASIO()
{
    m_NetIO.PollIO();
}

void Client::StopASIO()
{
    m_NetIO.StopIO();
}

void Client::OnServerMessage(uint8_t nHC, const uint8_t *pData, size_t nDataLen)
{
    // 1. update the time when last message received
    m_NetPackTick = SDL_GetTicks() * 1.0;
    if(nHC != SM_PING){
        g_DebugBoard->addLog("[%08.3f]%s", (float)(m_NetPackTick / 1000.0f), SMSGParam(nHC).Name().c_str());
    }

    m_ClientMonitor.SMProcMonitorList[nHC].RecvCount++;
    raii_timer stTimer(&(m_ClientMonitor.SMProcMonitorList[nHC].ProcTick));

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
        case SM_NOTIFYDEAD:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_NOTIFYDEAD(pData, nDataLen);
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
        case SM_MISS:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->Net_MISS(pData, nDataLen);
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

void Client::SwitchProcess()
{
    if(true
            && m_RequestProcess > PROCESSID_NONE
            && m_RequestProcess < PROCESSID_MAX){
        SwitchProcess((m_CurrentProcess ? m_CurrentProcess->ID() : PROCESSID_NONE), m_RequestProcess);
    }
    m_RequestProcess = PROCESSID_NONE;
}

void Client::SwitchProcess(int nNewID)
{
    SwitchProcess((m_CurrentProcess ? m_CurrentProcess->ID() : PROCESSID_NONE), nNewID);
}

void Client::SwitchProcess(int nOldID, int nNewID)
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
                            m_CurrentProcess = new ProcessSync();
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
                    case PROCESSID_NEW:
                        {
                            m_CurrentProcess = new ProcessNew();
                            break;
                        }
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

void Client::PrintMonitor() const
{
    g_Log->AddLog(LOGTYPE_DEBUG, "Client runs %" PRIu64 " msec", m_ClientTimer.diff_msec());
    for(size_t nIndex = 0; nIndex < SM_MAX; ++nIndex){
        uint64_t nProcTick  = m_ClientMonitor.SMProcMonitorList[nIndex].ProcTick / 1000000;
        uint64_t nRecvCount = m_ClientMonitor.SMProcMonitorList[nIndex].RecvCount;
        if(nRecvCount > 0){
            g_Log->AddLog(LOGTYPE_DEBUG, "%s: RecvCount = %" PRIu64 ", ProcTick = %" PRIu64 "msec", SMSGParam(nIndex).Name().c_str(), nRecvCount, nProcTick);
        }
    }
}
