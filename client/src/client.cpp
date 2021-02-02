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
#include "processnew.hpp"
#include "processrun.hpp"
#include "processlogo.hpp"
#include "processsync.hpp"
#include "pngtexoffdb.hpp"
#include "notifyboard.hpp"
#include "processlogin.hpp"
#include "servermsg.hpp"
#include "clientargparser.hpp"

extern Log *g_log;
extern XMLConf *g_XMLConf;
extern SDLDevice *g_sdlDevice;
extern NotifyBoard *g_notifyBoard;
extern ClientArgParser *g_clientArgParser;

Client::Client()
    : m_clientMonitor()
    , m_serverDelay( 0.00)
    , m_netPackTick(-1.00)
    , m_requestProcess(PROCESSID_NONE)
    , m_currentProcess(nullptr)
{
    InitView(10);
    g_sdlDevice->CreateMainWindow();
}

Client::~Client()
{}

void Client::processEvent()
{
    if(m_currentProcess){
        SDL_Event stEvent;
        while(SDL_PollEvent(&stEvent)){
            m_currentProcess->processEvent(stEvent);
        }
    }
}

void Client::mainLoop()
{
    SwitchProcess(PROCESSID_LOGO);
    initASIO();

    auto fDelayDraw   = (1000.0 / (1.0 * SYS_DEFFPS)) / 6.0;
    auto fDelayUpdate = (1000.0 / (1.0 * SYS_DEFFPS)) / 7.0;
    auto fDelayLoop   = (1000.0 / (1.0 * SYS_DEFFPS)) / 8.0;

    auto fLastDraw   = SDL_GetTicks() * 1.0;
    auto fLastUpdate = SDL_GetTicks() * 1.0;
    auto fLastLoop   = SDL_GetTicks() * 1.0;

    while(true){
        if(g_clientArgParser->enableClientMonitor){
            PrintMonitor();
        }

        SwitchProcess();
        if(true
                && m_currentProcess
                && m_currentProcess->ID() != PROCESSID_EXIT){

            if(m_netPackTick > 0.0){
                if(SDL_GetTicks() * 1.0 - m_netPackTick > 15.0 * 1000){
                    // std::exit(0);
                }
            }

            if(auto fCurrUpdate = 1.0 * SDL_GetTicks(); fCurrUpdate - fLastUpdate > fDelayUpdate){
                auto fPastUpdate = fCurrUpdate - fLastUpdate;
                fLastUpdate = fCurrUpdate;
                update(fPastUpdate);
            }

            if(auto fCurrDraw = 1.0 * SDL_GetTicks(); fCurrDraw - fLastDraw > fDelayDraw){
                fLastDraw = fCurrDraw;
                draw();
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
        m_netIO.poll();

        // everytime firstly try to process all pending events
        processEvent();

        double fCurrentMS = SDL_GetTicks() * 1.0;
        double fDelayDone = fCurrentMS - fStartDelayMS;

        if(fDelayDone >= fDelayMS){
            break;
        }

        // here we check the delay time
        // since SDL_Delay(0) may run into problem

        const auto nDelayMSCount = (Uint32)(std::lround((fDelayMS - fDelayDone) * 0.80));
        if(nDelayMSCount <= 0){
            break;
        }

        SDL_Delay(nDelayMSCount);
    }
}

void Client::initASIO()
{
    // this function will run in another thread
    // make sure there is no data race

    // TODO
    // may need lock here since g_XMLConf may used in main thread also
    const auto ipStr = []()-> std::string
    {
        if(!g_clientArgParser->serverIP.empty()){
            return g_clientArgParser->serverIP;
        }

        if(auto nodePtr = g_XMLConf->GetXMLNode("/Root/Network/Server/IP"); nodePtr && nodePtr->GetText()){
            return std::string(nodePtr->GetText());
        }
        return "127.0.0.1";
    }();

    const auto portStr = []()-> std::string
    {
        if(!g_clientArgParser->serverPort.empty()){
            return g_clientArgParser->serverPort;
        }

        if(auto nodePtr = g_XMLConf->GetXMLNode("/Root/Network/Server/Port"); nodePtr && nodePtr->GetText()){
            return std::string(nodePtr->GetText());
        }
        return "5000";
    }();

    m_netIO.start(ipStr.c_str(), portStr.c_str(), [this](uint8_t headCode, const uint8_t *pData, size_t nDataLen)
    {
        // core should handle on fully recieved message from the serer
        // previously there are two steps (HC, Body) seperately handled, error-prone
        onServerMessage(headCode, pData, nDataLen);
    });
}

void Client::onServerMessage(uint8_t headCode, const uint8_t *pData, size_t nDataLen)
{
    // 1. update the time when last message received
    m_netPackTick = SDL_GetTicks() * 1.0;
    if(headCode != SM_PING){
        sendSMsgLog(headCode);
    }

    m_clientMonitor.SMProcMonitorList[headCode].RecvCount++;
    raii_timer stTimer(&(m_clientMonitor.SMProcMonitorList[headCode].ProcTick));

    // 2. handle messages
    switch(headCode){
        case SM_UPDATEHP:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_UPDATEHP(pData, nDataLen);
                }
                break;
            }
        case SM_DEADFADEOUT:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_DEADFADEOUT(pData, nDataLen);
                }
                break;
            }
        case SM_NOTIFYDEAD:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_NOTIFYDEAD(pData, nDataLen);
                }
                break;
            }
        case SM_EXP:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_EXP(pData, nDataLen);
                }
                break;
            }
        case SM_MISS:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_MISS(pData, nDataLen);
                }
                break;
            }
        case SM_GOLD:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_GOLD(pData, nDataLen);
                }
                break;
            }
        case SM_CASTMAGIC:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_CASTMAGIC(pData, nDataLen);
                }
                break;
            }
        case SM_NPCXMLLAYOUT:
            {
                if(auto p = processRun(); p){
                    p->net_NPCXMLLAYOUT(pData, nDataLen);
                }
                break;
            }
        case SM_NPCSELL:
            {
                if(auto p = processRun(); p){
                    p->net_NPCSELL(pData, nDataLen);
                }
                break;
            }
        case SM_TEXT:
            {
                if(auto p = processRun(); p){
                    p->net_TEXT(pData, nDataLen);
                }
                break;
            }
        case SM_SHOWDROPITEM:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_SHOWDROPITEM(pData, nDataLen);
                }
                break;
            }
        case SM_PICKUPOK:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_PICKUPOK(pData, nDataLen);
                }
                break;
            }
        case SM_PING:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_PING(pData, nDataLen);
                }
                break;
            }
        case SM_SELLITEM:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_SELLITEM(pData, nDataLen);
                }
                break;
            }
        case SM_LOGINOK:
            {
                SwitchProcess(m_currentProcess->ID(), PROCESSID_RUN);
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_LOGINOK(pData, nDataLen);
                }else{
                    g_log->addLog(LOGTYPE_INFO, "failed to jump into main loop");
                }
                break;
            }
        case SM_CORECORD:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_CORECORD(pData, nDataLen);
                }
                break;
            }
        case SM_LOGINFAIL:
            {
                g_log->addLog(LOGTYPE_WARNING, "Login failed: ID = %d", (int)(((SMLoginFail *)(pData))->FailID));
                break;
            }
        case SM_ACTION:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_ACTION(pData, nDataLen);
                }
                break;
            }
        case SM_OFFLINE:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_OFFLINE(pData, nDataLen);
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
            && m_requestProcess > PROCESSID_NONE
            && m_requestProcess < PROCESSID_MAX){
        SwitchProcess((m_currentProcess ? m_currentProcess->ID() : PROCESSID_NONE), m_requestProcess);
    }
    m_requestProcess = PROCESSID_NONE;
}

void Client::SwitchProcess(int nNewID)
{
    SwitchProcess((m_currentProcess ? m_currentProcess->ID() : PROCESSID_NONE), nNewID);
}

void Client::SwitchProcess(int nOldID, int nNewID)
{
    delete m_currentProcess;
    m_currentProcess = nullptr;

    switch(nOldID)
    {
        case PROCESSID_NONE:
            {
                switch(nNewID)
                {
                    case PROCESSID_LOGO:
                        {
                            // on initialization
                            m_currentProcess = new ProcessLogo();
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
                            m_currentProcess = new ProcessSync();
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
                            m_currentProcess = new ProcessLogin();
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
                            m_currentProcess = new ProcessNew();
                            break;
                        }
                    case PROCESSID_RUN:
                        {
                            m_currentProcess = new ProcessRun();
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

void Client::sendCMsgLog(uint8_t headCode)
{
    g_notifyBoard->addLog(u8"[%08.3f] ← %s", (float)(SDL_GetTicks()) / 1000.0f, ClientMsg(headCode).name().c_str());
}

void Client::sendSMsgLog(uint8_t headCode)
{
    g_notifyBoard->addLog(u8"[%08.3f] → %s", (float)(SDL_GetTicks()) / 1000.0f, ServerMsg(headCode).name().c_str());
}

void Client::PrintMonitor() const
{
    g_log->addLog(LOGTYPE_DEBUG, "Client runs %" PRIu64 " msec", m_clientTimer.diff_msec());
    for(size_t nIndex = 0; nIndex < SM_MAX; ++nIndex){
        uint64_t nProcTick  = m_clientMonitor.SMProcMonitorList[nIndex].ProcTick / 1000000;
        uint64_t nRecvCount = m_clientMonitor.SMProcMonitorList[nIndex].RecvCount;
        if(nRecvCount > 0){
            g_log->addLog(LOGTYPE_DEBUG, "%s: RecvCount = %" PRIu64 ", ProcTick = %" PRIu64 "msec", ServerMsg(nIndex).name().c_str(), nRecvCount, nProcTick);
        }
    }
}
