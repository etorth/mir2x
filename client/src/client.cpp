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
#include "processrun.hpp"
#include "processlogo.hpp"
#include "processsync.hpp"
#include "processcreateaccount.hpp"
#include "pngtexoffdb.hpp"
#include "notifyboard.hpp"
#include "buildconfig.hpp"
#include "processlogin.hpp"
#include "clientargparser.hpp"

extern Log *g_log;
extern XMLConf *g_xmlConf;
extern SDLDevice *g_sdlDevice;
extern NotifyBoard *g_notifyBoard;
extern ClientArgParser *g_clientArgParser;

Client::Client()
    : m_clientMonitor()
    , m_serverDelay( 0.00)
    , m_netPackTick(-1.00)
{
    InitView(10);
    g_sdlDevice->createMainWindow();
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
    switchProcess(PROCESSID_LOGO);
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

        if(!(m_currentProcess && (m_currentProcess->id() != PROCESSID_EXIT))){
            break;
        }

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
        eventDelay(fDelayLoop - fPastLoop);
        switchProcess();
    }
}

void Client::eventDelay(double fDelayMS)
{
    const double fStartDelayMS = SDL_GetTicks() * 1.0;
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
    // may need lock here since g_xmlConf may used in main thread also
    const auto ipStr = []()-> std::string
    {
        if(!g_clientArgParser->serverIP.empty()){
            return g_clientArgParser->serverIP;
        }

        if(auto nodePtr = g_xmlConf->getXMLNode("/root/network/server/ip"); nodePtr && nodePtr->GetText()){
            return std::string(nodePtr->GetText());
        }
        return "127.0.0.1";
    }();

    const auto portStr = []()-> std::string
    {
        if(!g_clientArgParser->serverPort.empty()){
            return g_clientArgParser->serverPort;
        }

        if(auto nodePtr = g_xmlConf->getXMLNode("/root/network/server/port"); nodePtr && nodePtr->GetText()){
            return std::string(nodePtr->GetText());
        }
        return "5000";
    }();

    m_netIO.start(ipStr.c_str(), portStr.c_str(), [this](uint8_t headCode, const uint8_t *pData, size_t nDataLen)
    {
        // core should handle on fully recieved message from the serer
        // previously there are two steps (HC, Body) seperately handled, error-prone
        onServerMessage(headCode, pData, nDataLen);
        switchProcess();
    });
}

void Client::onServerMessage(uint8_t headCode, const uint8_t *buf, size_t bufSize)
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
        case SM_LOGINOK:
            {
                if(auto proc = (ProcessLogin *)(ProcessValid(PROCESSID_LOGIN))){
                    proc->net_LOGINOK(buf, bufSize);
                }
                break;
            }
        case SM_STARTGAMESCENE:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_STARTGAMESCENE(buf, bufSize);
                }
                break;
            }
        case SM_CREATEACCOUNTOK:
            {
                if(auto proc = (ProcessCreateAccount *)(ProcessValid(PROCESSID_CREATEACCOUNT))){
                    proc->net_CREATEACCOUNTOK(buf, bufSize);
                }
                break;
            }
        case SM_CREATEACCOUNTERROR:
            {
                if(auto proc = (ProcessCreateAccount *)(ProcessValid(PROCESSID_CREATEACCOUNT))){
                    proc->net_CREATEACCOUNTERROR(buf, bufSize);
                }
                break;
            }
        case SM_HEALTH:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_HEALTH(buf, bufSize);
                }
                break;
            }
        case SM_NEXTSTRIKE:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_NEXTSTRIKE(buf, bufSize);
                }
                break;
            }
        case SM_DEADFADEOUT:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_DEADFADEOUT(buf, bufSize);
                }
                break;
            }
        case SM_NOTIFYDEAD:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_NOTIFYDEAD(buf, bufSize);
                }
                break;
            }
        case SM_EXP:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_EXP(buf, bufSize);
                }
                break;
            }
        case SM_BUFF:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_BUFF(buf, bufSize);
                }
                break;
            }
        case SM_MISS:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_MISS(buf, bufSize);
                }
                break;
            }
        case SM_GOLD:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_GOLD(buf, bufSize);
                }
                break;
            }
        case SM_INVOPCOST:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_INVOPCOST(buf, bufSize);
                }
                break;
            }
        case SM_STRIKEGRID:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_STRIKEGRID(buf, bufSize);
                }
                break;
            }
        case SM_PLAYERWLDESP:
            {
                if(auto runPtr = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    runPtr->net_PLAYERWLDESP(buf, bufSize);
                }
                break;
            }
        case SM_CASTMAGIC:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_CASTMAGIC(buf, bufSize);
                }
                break;
            }
        case SM_NPCXMLLAYOUT:
            {
                if(auto p = processRun(); p){
                    p->net_NPCXMLLAYOUT(buf, bufSize);
                }
                break;
            }
        case SM_NPCSELL:
            {
                if(auto p = processRun(); p){
                    p->net_NPCSELL(buf, bufSize);
                }
                break;
            }
        case SM_STARTINVOP:
            {
                if(auto p = processRun(); p){
                    p->net_STARTINVOP(buf, bufSize);
                }
                break;
            }
        case SM_STARTINPUT:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_STARTINPUT(buf, bufSize);
                }
                break;
            }
        case SM_TEXT:
            {
                if(auto p = processRun(); p){
                    p->net_TEXT(buf, bufSize);
                }
                break;
            }
        case SM_SHOWSECUREDITEMLIST:
            {
                if(auto p = processRun(); p){
                    p->net_SHOWSECUREDITEMLIST(buf, bufSize);
                }
                break;
            }
        case SM_PLAYERNAME:
            {
                if(auto p = processRun(); p){
                    p->net_PLAYERNAME(buf, bufSize);
                }
                break;
            }
        case SM_BUYSUCCEED:
            {
                if(auto p = processRun(); p){
                    p->net_BUYSUCCEED(buf, bufSize);
                }
                break;
            }
        case SM_BUYERROR:
            {
                if(auto p = processRun(); p){
                    p->net_BUYERROR(buf, bufSize);
                }
                break;
            }
        case SM_GROUNDITEMIDLIST:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_GROUNDITEMIDLIST(buf, bufSize);
                }
                break;
            }
        case SM_GROUNDFIREWALLLIST:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_GROUNDFIREWALLLIST(buf, bufSize);
                }
                break;
            }
        case SM_PICKUPERROR:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_PICKUPERROR(buf, bufSize);
                }
                break;
            }
        case SM_PING:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_PING(buf, bufSize);
                }
                break;
            }
        case SM_SELLITEMLIST:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_SELLITEMLIST(buf, bufSize);
                }
                break;
            }
        case SM_RUNTIMECONFIG:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_RUNTIMECONFIG(buf, bufSize);
                }
                break;
            }
        case SM_LEARNEDMAGICLIST:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_LEARNEDMAGICLIST(buf, bufSize);
                }
                break;
            }
        case SM_CORECORD:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_CORECORD(buf, bufSize);
                }
                break;
            }
        case SM_ACTION:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_ACTION(buf, bufSize);
                }
                break;
            }
        case SM_UPDATEITEM:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_UPDATEITEM(buf, bufSize);
                }
                break;
            }
        case SM_INVENTORY:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_INVENTORY(buf, bufSize);
                }
                break;
            }
        case SM_BELT:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_BELT(buf, bufSize);
                }
                break;
            }
        case SM_REMOVEITEM:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_REMOVEITEM(buf, bufSize);
                }
                break;
            }
        case SM_REMOVESECUREDITEM:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_REMOVESECUREDITEM(buf, bufSize);
                }
                break;
            }
        case SM_OFFLINE:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_OFFLINE(buf, bufSize);
                }
                break;
            }
        case SM_BUILDVERSION:
            {
                if(!g_clientArgParser->disableVersionCheck){
                    if(const auto smBV = ServerMsg::conv<SMBuildVersion>(buf); smBV.version.as_sv() != getBuildSignature()){
                        throw fflerror("client/server version mismatches, client: %s, server: %s", getBuildSignature(), to_cstr(smBV.version));
                    }
                }
                break;
            }
        case SM_EQUIPWEAR:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_EQUIPWEAR(buf, bufSize);
                }
                break;
            }
        case SM_EQUIPWEARERROR:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_EQUIPWEARERROR(buf, bufSize);
                }
                break;
            }
        case SM_GRABWEAR:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_GRABWEAR(buf, bufSize);
                }
                break;
            }
        case SM_GRABWEARERROR:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_GRABWEARERROR(buf, bufSize);
                }
                break;
            }
        case SM_EQUIPBELT:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_EQUIPBELT(buf, bufSize);
                }
                break;
            }
        case SM_EQUIPBELTERROR:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_EQUIPBELTERROR(buf, bufSize);
                }
                break;
            }
        case SM_GRABBELT:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_GRABBELT(buf, bufSize);
                }
                break;
            }
        case SM_GRABBELTERROR:
            {
                if(auto proc = (ProcessRun *)(ProcessValid(PROCESSID_RUN))){
                    proc->net_GRABBELTERROR(buf, bufSize);
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

void Client::switchProcess()
{
    if(m_requestProcess >= PROCESSID_BEGIN && m_requestProcess < PROCESSID_END){
        switchProcess(m_currentProcess ? m_currentProcess->id() : PROCESSID_NONE, m_requestProcess);
    }
    m_requestProcess = PROCESSID_NONE;
}

void Client::switchProcess(int newID)
{
    switchProcess((m_currentProcess ? m_currentProcess->id() : PROCESSID_NONE), newID);
}

void Client::switchProcess(int oldID, int newID)
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
                    case PROCESSID_CREATEACCOUNT:
                        {
                            m_currentProcess = std::make_unique<ProcessCreateAccount>();
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
        case PROCESSID_CREATEACCOUNT:
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
