/*
 * =====================================================================================
 *
 *       Filename: monoserver.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 05/08/2016 15:00:56
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
#include <cstdarg>
#include <cstdlib>

#include "log.hpp"
#include "taskhub.hpp"
#include "eventtaskhub.hpp"
#include "message.hpp"
#include "database.hpp"
#include "mainwindow.hpp"
#include "monoserver.hpp"

#include "threadpn.hpp"
#include "servicecore.hpp"
#include "databaseconfigurewindow.hpp"

MonoServer::MonoServer()
    : SyncDriver()
    , m_LogBuf(nullptr)
    , m_LogBufSize(0)
    , m_SessionHub(nullptr)
    , m_ObjectUID(1)
{
    m_StartTime = std::chrono::system_clock::now();
}

MonoServer::~MonoServer()
{
}

void MonoServer::ExtendLogBuf(size_t nNewSize)
{
    if(nNewSize > m_LogBufSize){
        delete[] m_LogBuf;
        size_t nNewSize8 = ((nNewSize + 7) / 8) * 8;
        m_LogBuf     = new char[nNewSize8];
        m_LogBufSize = nNewSize8;
    }
}

void MonoServer::AddLog(const std::array<std::string, 4> &stLogDesc, const char *szLogFormat, ...)
{
    std::lock_guard<std::mutex> stGuard(m_LogLock);

    extern Log *g_Log;
    extern MainWindow *g_MainWindow;

    va_list ap;
    int nMaxParseCount = 0;
    int nLogType = std::atoi(stLogDesc[0].c_str());

    ExtendLogBuf(128);

    // actually it only needs 2 rounds at most
    while(10 > nMaxParseCount++){

        va_start(ap, szLogFormat);
        int nRes = std::vsnprintf(m_LogBuf, m_LogBufSize, szLogFormat, ap);
        va_end(ap);

        if(nRes > -1 && (size_t)nRes < m_LogBufSize){
            // additional '\0' takes one char, everything works
            if(nLogType != Log::LOGTYPEV_DEBUG){
                g_MainWindow->AddLog(nLogType, m_LogBuf);
            }
            g_Log->AddLog(stLogDesc, m_LogBuf);
            return;
        }else if(nRes < 0){
            // error occurs in parsing log
            break;
        }else{
            // we need a larger buffer
            ExtendLogBuf(nRes + 1);
        }
    }

    const char *szLogError = "MonoServer::AddLog(): Error in parsing log message";
    g_MainWindow->AddLog(3, szLogError);
    g_Log->AddLog(stLogDesc, szLogError);
}

void MonoServer::CreateDBConnection()
{
    extern DatabaseConfigureWindow *g_DatabaseConfigureWindow;

    // delete m_UserInfoDB;

    m_DBConnection = new DBConnection(
            g_DatabaseConfigureWindow->DatabaseIP(),
            g_DatabaseConfigureWindow->UserName(),
            g_DatabaseConfigureWindow->Password(),
            g_DatabaseConfigureWindow->DatabaseName(),
            g_DatabaseConfigureWindow->DatabasePort());

    if(m_DBConnection->Valid()){
        AddLog(LOGTYPE_INFO, "Connect to Database (%s:%d) successfully", 
                g_DatabaseConfigureWindow->DatabaseIP(),
                g_DatabaseConfigureWindow->DatabasePort());
    }else{
        AddLog(LOGTYPE_WARNING, "Can't connect to Database (%s:%d)", 
                g_DatabaseConfigureWindow->DatabaseIP(),
                g_DatabaseConfigureWindow->DatabasePort());
    }
}

void MonoServer::Launch()
{
    // 1. create db connection
    delete m_DBConnection;
    CreateDBConnection();

    // 2. load monster info
    InitMonsterRace();
    InitMonsterItem();

    // 3. start service core
    delete m_ServiceCore;
    m_ServiceCore = new ServiceCore();
    m_ServiceCoreAddress = m_ServiceCore->Activate();

    // 4. start session hub
    extern ServerConfigureWindow *g_ServerConfigureWindow;
    int nPort = g_ServerConfigureWindow->Port();
    auto fnOnReadHC = [this](uint8_t nMsgHC, Session *pSession){ OnReadHC(nMsgHC, pSession); };

    delete m_SessionHub;
    m_SessionHub = new SessionHub(nPort, m_ServiceCoreAddress, fnOnReadHC);
    m_SessionHub->Launch();

    extern EventTaskHub *g_EventTaskHub;
    g_EventTaskHub->Launch();

    AddMonster(1, 1, 765, 573, false);

    // TODO
    // dead lock when there is too many monsters???
}

void MonoServer::OnReadHC(uint8_t nMsgHC, Session *pSession)
{
    switch(nMsgHC){
        case CM_LOGIN:
            {
                OnLogin(pSession);
                break;
            }
        case CM_BROADCAST:
            {
                // OnBroadcast(pSession);
                break;
            }
        default:
            {
                OnForward(nMsgHC, pSession);
                break;
            }
    }
}

uint32_t MonoServer::GetTickCount()
{
    // TODO
    // make it more simple
    return (uint32_t)std::chrono::duration_cast<
        std::chrono::milliseconds>(std::chrono::system_clock::now() - m_StartTime).count();
}

void MonoServer::Restart()
{
    exit(0);
}
