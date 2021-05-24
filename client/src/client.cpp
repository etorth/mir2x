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

#include <set>
#include <future>
#include <thread>
#include <cstring>
#include <cinttypes>

#include "log.hpp"
#include "client.hpp"
#include "xmlconf.hpp"
#include "initview.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "fontexdb.hpp"
#include "sdldevice.hpp"
#include "servermsg.hpp"
#include "processnew.hpp"
#include "processrun.hpp"
#include "processlogo.hpp"
#include "processsync.hpp"
#include "pngtexoffdb.hpp"
#include "notifyboard.hpp"
#include "buildconfig.hpp"
#include "processlogin.hpp"
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

        if(auto nodePtr = g_XMLConf->getXMLNode("/Root/Network/Server/IP"); nodePtr && nodePtr->GetText()){
            return std::string(nodePtr->GetText());
        }
        return "127.0.0.1";
    }();

    const auto portStr = []()-> std::string
    {
        if(!g_clientArgParser->serverPort.empty()){
            return g_clientArgParser->serverPort;
        }

        if(auto nodePtr = g_XMLConf->getXMLNode("/Root/Network/Server/Port"); nodePtr && nodePtr->GetText()){
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

    m_clientMonitor.SMProcMonitorList[headCode].recvCount++;
    raii_timer stTimer(&(m_clientMonitor.SMProcMonitorList[headCode].procTick));

    // 2. handle messages
    switch(headCode){
        case SM_ACCOUNT:
            {
                if(auto proc = (ProcessNew *)(ProcessValid(PROCESSID_NEW))){
                    proc->net_ACCOUNT(pData, nDataLen);
                }
                break;
            }
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
        case SM_BUFF:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_BUFF(pData, nDataLen);
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
        case SM_STRIKEGRID:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_STRIKEGRID(pData, nDataLen);
                }
                break;
            }
        case SM_PLAYERWLDESP:
            {
                if(auto runPtr = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    runPtr->net_PLAYERWLDESP(pData, nDataLen);
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
        case SM_PLAYERNAME:
            {
                if(auto p = processRun(); p){
                    p->net_PLAYERNAME(pData, nDataLen);
                }
                break;
            }
        case SM_BUYSUCCEED:
            {
                if(auto p = processRun(); p){
                    p->net_BUYSUCCEED(pData, nDataLen);
                }
                break;
            }
        case SM_BUYERROR:
            {
                if(auto p = processRun(); p){
                    p->net_BUYERROR(pData, nDataLen);
                }
                break;
            }
        case SM_GROUNDITEMIDLIST:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_GROUNDITEMIDLIST(pData, nDataLen);
                }
                break;
            }
        case SM_GROUNDFIREWALLLIST:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_GROUNDFIREWALLLIST(pData, nDataLen);
                }
                break;
            }
        case SM_PICKUPERROR:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_PICKUPERROR(pData, nDataLen);
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
        case SM_SELLITEMLIST:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_SELLITEMLIST(pData, nDataLen);
                }
                break;
            }
        case SM_LOGINOK:
            {
                SwitchProcess(m_currentProcess->ID(), PROCESSID_RUN);
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_LOGINOK(pData, nDataLen);
                }
                else{
                    throw fflerror("failed to switch to ProcessRun");
                }
                break;
            }
        case SM_RUNTIMECONFIG:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_RUNTIMECONFIG(pData, nDataLen);
                }
                break;
            }
        case SM_LEARNEDMAGICLIST:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_LEARNEDMAGICLIST(pData, nDataLen);
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
                g_log->addLog(LOGTYPE_WARNING, "Login failed: error = %llu", to_llu(ServerMsg::conv<SMLoginFail>(pData).error));
                break;
            }
        case SM_ACTION:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_ACTION(pData, nDataLen);
                }
                break;
            }
        case SM_UPDATEITEM:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_UPDATEITEM(pData, nDataLen);
                }
                break;
            }
        case SM_INVENTORY:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_INVENTORY(pData, nDataLen);
                }
                break;
            }
        case SM_BELT:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_BELT(pData, nDataLen);
                }
                break;
            }
        case SM_REMOVEITEM:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_REMOVEITEM(pData, nDataLen);
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
        case SM_BUILDVERSION:
            {
                if(!g_clientArgParser->disableVersionCheck){
                    const auto smBV = ServerMsg::conv<SMBuildVersion>(pData);
                    if(std::strcmp(smBV.version, getBuildSignature())){
                        throw fflerror("client/server version mismatches, client: %s, server: %s", getBuildSignature(), smBV.version);
                    }
                }
                break;
            }
        case SM_EQUIPWEAR:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_EQUIPWEAR(pData, nDataLen);
                }
                break;
            }
        case SM_EQUIPWEARERROR:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_EQUIPWEARERROR(pData, nDataLen);
                }
                break;
            }
        case SM_GRABWEAR:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_GRABWEAR(pData, nDataLen);
                }
                break;
            }
        case SM_GRABWEARERROR:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_GRABWEARERROR(pData, nDataLen);
                }
                break;
            }
        case SM_EQUIPBELT:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_EQUIPBELT(pData, nDataLen);
                }
                break;
            }
        case SM_EQUIPBELTERROR:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_EQUIPBELTERROR(pData, nDataLen);
                }
                break;
            }
        case SM_GRABBELT:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_GRABBELT(pData, nDataLen);
                }
                break;
            }
        case SM_GRABBELTERROR:
            {
                if(auto pRun = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    pRun->net_GRABBELTERROR(pData, nDataLen);
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
            && m_requestProcess >= PROCESSID_BEGIN
            && m_requestProcess <  PROCESSID_END){
        SwitchProcess((m_currentProcess ? m_currentProcess->ID() : PROCESSID_NONE), m_requestProcess);
    }
    m_requestProcess = PROCESSID_NONE;
}

void Client::SwitchProcess(int nNewID)
{
    SwitchProcess((m_currentProcess ? m_currentProcess->ID() : PROCESSID_NONE), nNewID);
}

void Client::SwitchProcess(int oldID, int newID)
{
    m_currentProcess.reset();
    switch(oldID){
        case PROCESSID_NONE:
            {
                switch(newID)
                {
                    case PROCESSID_LOGO:
                        {
                            // on initialization
                            m_currentProcess = std::make_unique<ProcessLogo>();
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
                switch(newID)
                {
                    case PROCESSID_SYRC:
                        {
                            // on initialization
                            m_currentProcess = std::make_unique<ProcessSync>();
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
                switch(newID)
                {
                    case PROCESSID_LOGIN:
                        {
                            m_currentProcess = std::make_unique<ProcessLogin>();
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
                switch(newID){
                    case PROCESSID_NEW:
                        {
                            m_currentProcess = std::make_unique<ProcessNew>();
                            break;
                        }
                    case PROCESSID_RUN:
                        {
                            m_currentProcess = std::make_unique<ProcessRun>();
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        case PROCESSID_NEW:
            {
                switch(newID){
                    case PROCESSID_LOGIN:
                        {
                            m_currentProcess = std::make_unique<ProcessLogin>();
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
    g_log->addLog(LOGTYPE_DEBUG, "Client runs %llu msec", to_llu(m_clientTimer.diff_msec()));
    for(size_t nIndex = 0; nIndex < SM_END; ++nIndex){
        uint64_t nProcTick  = m_clientMonitor.SMProcMonitorList[nIndex].procTick / 1000000;
        uint64_t nRecvCount = m_clientMonitor.SMProcMonitorList[nIndex].recvCount;
        if(nRecvCount > 0){
            g_log->addLog(LOGTYPE_DEBUG, "%s: recvCount = %llu, procTick = %llumsec", ServerMsg(nIndex).name().c_str(), to_llu(nRecvCount), to_llu(nProcTick));
        }
    }
}
