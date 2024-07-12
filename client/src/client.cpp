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
#include "processselectchar.hpp"
#include "processcreatechar.hpp"
#include "processcreateaccount.hpp"
#include "processchangepassword.hpp"
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
            switchProcess();
        }
    }
}

void Client::mainLoop()
{
    switchProcess(PROCESSID_LOGO);
    initASIO();

    const auto fDelayDraw   = (1000.0 / (1.0 * SYS_DEFFPS)) / 6.0;
    const auto fDelayUpdate = (1000.0 / (1.0 * SYS_DEFFPS)) / 7.0;
    const auto fDelayLoop   = (1000.0 / (1.0 * SYS_DEFFPS)) / 8.0;

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
    }
}

void Client::eventDelay(double fDelayMS)
{
    const double fStartDelayMS = SDL_GetTicks() * 1.0;
    while(true){
        m_netIO.poll();
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

    m_netIO.start(ipStr.c_str(), portStr.c_str(), [this](uint8_t headCode, const uint8_t *pData, size_t nDataLen, uint64_t respID)
    {
        // core should handle on fully recieved message from the serer
        // previously there are two steps (HC, Body) seperately handled, error-prone

        if(respID){
            if(auto p = m_respHandlers.find(respID); p != m_respHandlers.end()){
                p->second.handler(headCode, pData, nDataLen);
                m_respHandlers.erase(p);
            }
            else{
                throw fflerror("no handler found for response id %llu", to_llu(respID));
            }
        }
        else{
            onServerMessage(headCode, pData, nDataLen);
        }
        switchProcess();
    });
}

void Client::onServerMessage(uint8_t headCode, const uint8_t *buf, size_t bufSize)
{
    m_netPackTick = SDL_GetTicks() * 1.0;
    if(headCode != SM_PING){
        sendSMsgLog(headCode);
    }

    m_clientMonitor.SMProcMonitorList[headCode].recvCount++;
    raii_timer stTimer(&(m_clientMonitor.SMProcMonitorList[headCode].procTick));

    switch(headCode){
        case SM_BUILDVERSION:
            {
                if(!g_clientArgParser->disableVersionCheck){
                    if(const auto smBV = ServerMsg::conv<SMBuildVersion>(buf); smBV.version.as_sv() != getBuildSignature()){
                        throw fflerror("client/server version mismatches, client: %s, server: %s", getBuildSignature(), smBV.version.as_sv().data());
                    }
                }
                break;
            }

#define _INSTALL_SM_HANDLER(ProcessType, smType) case smType: {if(auto proc = getProcess<ProcessType>()){ proc->on_##smType(buf, bufSize); } break; }

        _INSTALL_SM_HANDLER(ProcessLogin, SM_LOGINERROR)
        _INSTALL_SM_HANDLER(ProcessLogin, SM_LOGINOK)

        _INSTALL_SM_HANDLER(ProcessCreateChar, SM_CREATECHARERROR)
        _INSTALL_SM_HANDLER(ProcessCreateChar, SM_CREATECHAROK)

        _INSTALL_SM_HANDLER(ProcessCreateAccount, SM_CREATEACCOUNTOK)
        _INSTALL_SM_HANDLER(ProcessCreateAccount, SM_CREATEACCOUNTERROR)

        _INSTALL_SM_HANDLER(ProcessChangePassword, SM_CHANGEPASSWORDERROR)
        _INSTALL_SM_HANDLER(ProcessChangePassword, SM_CHANGEPASSWORDOK)

        _INSTALL_SM_HANDLER(ProcessSelectChar, SM_DELETECHARERROR)
        _INSTALL_SM_HANDLER(ProcessSelectChar, SM_DELETECHAROK)
        _INSTALL_SM_HANDLER(ProcessSelectChar, SM_ONLINEERROR)
        _INSTALL_SM_HANDLER(ProcessSelectChar, SM_ONLINEOK)
        _INSTALL_SM_HANDLER(ProcessSelectChar, SM_QUERYCHARERROR)
        _INSTALL_SM_HANDLER(ProcessSelectChar, SM_QUERYCHAROK)

        _INSTALL_SM_HANDLER(ProcessRun, SM_ACTION)
        _INSTALL_SM_HANDLER(ProcessRun, SM_BELT)
        _INSTALL_SM_HANDLER(ProcessRun, SM_BUFF)
        _INSTALL_SM_HANDLER(ProcessRun, SM_BUFFIDLIST)
        _INSTALL_SM_HANDLER(ProcessRun, SM_BUYERROR)
        _INSTALL_SM_HANDLER(ProcessRun, SM_BUYSUCCEED)
        _INSTALL_SM_HANDLER(ProcessRun, SM_CASTMAGIC)
        _INSTALL_SM_HANDLER(ProcessRun, SM_CORECORD)
        _INSTALL_SM_HANDLER(ProcessRun, SM_DEADFADEOUT)
        _INSTALL_SM_HANDLER(ProcessRun, SM_EQUIPBELT)
        _INSTALL_SM_HANDLER(ProcessRun, SM_EQUIPBELTERROR)
        _INSTALL_SM_HANDLER(ProcessRun, SM_EQUIPWEAR)
        _INSTALL_SM_HANDLER(ProcessRun, SM_EQUIPWEARERROR)
        _INSTALL_SM_HANDLER(ProcessRun, SM_EXP)
        _INSTALL_SM_HANDLER(ProcessRun, SM_FRIENDLIST)
        _INSTALL_SM_HANDLER(ProcessRun, SM_CHATMESSAGELIST)
        _INSTALL_SM_HANDLER(ProcessRun, SM_GOLD)
        _INSTALL_SM_HANDLER(ProcessRun, SM_GRABBELT)
        _INSTALL_SM_HANDLER(ProcessRun, SM_GRABBELTERROR)
        _INSTALL_SM_HANDLER(ProcessRun, SM_GRABWEAR)
        _INSTALL_SM_HANDLER(ProcessRun, SM_GRABWEARERROR)
        _INSTALL_SM_HANDLER(ProcessRun, SM_GROUNDFIREWALLLIST)
        _INSTALL_SM_HANDLER(ProcessRun, SM_GROUNDITEMIDLIST)
        _INSTALL_SM_HANDLER(ProcessRun, SM_HEALTH)
        _INSTALL_SM_HANDLER(ProcessRun, SM_INVENTORY)
        _INSTALL_SM_HANDLER(ProcessRun, SM_INVOPCOST)
        _INSTALL_SM_HANDLER(ProcessRun, SM_LEARNEDMAGICLIST)
        _INSTALL_SM_HANDLER(ProcessRun, SM_MISS)
        _INSTALL_SM_HANDLER(ProcessRun, SM_NEXTSTRIKE)
        _INSTALL_SM_HANDLER(ProcessRun, SM_NOTIFYDEAD)
        _INSTALL_SM_HANDLER(ProcessRun, SM_NPCSELL)
        _INSTALL_SM_HANDLER(ProcessRun, SM_NPCXMLLAYOUT)
        _INSTALL_SM_HANDLER(ProcessRun, SM_OFFLINE)
        _INSTALL_SM_HANDLER(ProcessRun, SM_PICKUPERROR)
        _INSTALL_SM_HANDLER(ProcessRun, SM_PING)
        _INSTALL_SM_HANDLER(ProcessRun, SM_PLAYERCONFIG)
        _INSTALL_SM_HANDLER(ProcessRun, SM_PLAYERNAME)
        _INSTALL_SM_HANDLER(ProcessRun, SM_PLAYERWLDESP)
        _INSTALL_SM_HANDLER(ProcessRun, SM_QUESTDESPLIST)
        _INSTALL_SM_HANDLER(ProcessRun, SM_QUESTDESPUPDATE)
        _INSTALL_SM_HANDLER(ProcessRun, SM_REMOVEITEM)
        _INSTALL_SM_HANDLER(ProcessRun, SM_REMOVESECUREDITEM)
        _INSTALL_SM_HANDLER(ProcessRun, SM_SELLITEMLIST)
        _INSTALL_SM_HANDLER(ProcessRun, SM_SHOWSECUREDITEMLIST)
        _INSTALL_SM_HANDLER(ProcessRun, SM_STARTGAMESCENE)
        _INSTALL_SM_HANDLER(ProcessRun, SM_STARTINPUT)
        _INSTALL_SM_HANDLER(ProcessRun, SM_STARTINVOP)
        _INSTALL_SM_HANDLER(ProcessRun, SM_STRIKEGRID)
        _INSTALL_SM_HANDLER(ProcessRun, SM_TEAMCANDIDATE)
        _INSTALL_SM_HANDLER(ProcessRun, SM_TEAMMEMBERLIST)
        _INSTALL_SM_HANDLER(ProcessRun, SM_TEXT)
        _INSTALL_SM_HANDLER(ProcessRun, SM_UPDATEITEM)
        _INSTALL_SM_HANDLER(ProcessRun, SM_CREATECHATGROUP)

#undef _INSTALL_SM_HANDLER

        default:
            {
                throw fflerror("no handler registered for server message %d", to_d(headCode));
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
                    case PROCESSID_SELECTCHAR:
                        {
                            m_currentProcess = std::make_unique<ProcessSelectChar>();
                            break;
                        }
                    case PROCESSID_CREATEACCOUNT:
                        {
                            m_currentProcess = std::make_unique<ProcessCreateAccount>();
                            break;
                        }
                    case PROCESSID_CHANGEPASSWORD:
                        {
                            m_currentProcess = std::make_unique<ProcessChangePassword>();
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
                break;
            }
        case PROCESSID_CHANGEPASSWORD:
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
                break;
            }
        case PROCESSID_SELECTCHAR:
            {
                switch(newID){
                    case PROCESSID_RUN:
                        {
                            fflassert(m_smOOK.has_value());
                            m_currentProcess = std::make_unique<ProcessRun>(m_smOOK.value());
                            break;
                        }
                    case PROCESSID_CREATECHAR:
                        {
                            m_currentProcess = std::make_unique<ProcessCreateChar>();
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        case PROCESSID_CREATECHAR:
            {
                switch(newID){
                    case PROCESSID_SELECTCHAR:
                        {
                            m_currentProcess = std::make_unique<ProcessSelectChar>();
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
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
